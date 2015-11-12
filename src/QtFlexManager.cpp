#include "QtFlexManager.h"
#include "QtFlexWidget.h"
#include "QtDockWidget.h"
#include "QtDockSite.h"
#include <QtCore/QAbstractNativeEventFilter>
#include <QtCore/QVariant>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

int Flex::Update = QEvent::registerEventType();

#ifdef Q_OS_WIN
WId topLevelWindowAt(QWidget* widget, const QPoint& pos)
{
    WId hTmp = 0;
    HWND hWnd = GetWindow(reinterpret_cast<HWND>(widget->effectiveWinId()), GW_HWNDNEXT);
    while (hWnd != nullptr && reinterpret_cast<HWND>(hTmp) == nullptr)
    {
        POINT pnt = { pos.x(), pos.y() };
        ScreenToClient(hWnd, &pnt);
        hTmp = reinterpret_cast<WId>(ChildWindowFromPoint(hWnd, pnt));
        hWnd = GetWindow(hWnd, GW_HWNDNEXT);
    }
    return reinterpret_cast<WId>(GetAncestor(reinterpret_cast<HWND>(hTmp), GA_ROOT));
}
#endif

DockSite* getDockSite(QWidget* widget)
{
    for (DockSite* site = nullptr; widget && !site; widget = widget->parentWidget())
    {
        if ((site = qobject_cast<DockSite*>(widget)) != nullptr)
        {
            return site;
        }
    }
    return nullptr;
}

class FlexManagerImpl : public QAbstractNativeEventFilter
{
public:
    FlexManagerImpl() : _ready(false)
    {
    }

public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

public:
    QList<FlexWidget*> _flexWidgets;
    QList<DockWidget*> _dockWidgets;
    bool _ready;
    QList<QIcon> _buttonIcons;
};

bool FlexManagerImpl::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    return false;
}

FlexManager::FlexManager() : impl(new FlexManagerImpl)
{
    Q_ASSERT(qApp != nullptr);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), SLOT(on_app_focusChanged(QWidget*, QWidget*)));
    connect(this, SIGNAL(guiderHover(FlexWidget*, QWidget*)), SLOT(on_flexWidget_guiderHover(FlexWidget*, QWidget*)));
    connect(this, SIGNAL(guiderShow(FlexWidget*, QWidget*)), SLOT(on_flexWidget_guiderShow(FlexWidget*, QWidget*)));
    connect(this, SIGNAL(guiderHide(FlexWidget*, QWidget*)), SLOT(on_flexWidget_guiderHide(FlexWidget*, QWidget*)));
    connect(this, SIGNAL(guiderDrop(FlexWidget*, DockWidget*)), SLOT(on_flexWidget_guiderDrop(FlexWidget*, DockWidget*)));
    connect(this, SIGNAL(guiderDrop(FlexWidget*, FlexWidget*)), SLOT(on_flexWidget_guiderDrop(FlexWidget*, FlexWidget*)));
    QPixmap extentsPixmap(":/Resources/extents.png");
    for (int i = 0; i < 3; i++)
    {
        QIcon icon;
        icon.addPixmap(extentsPixmap.copy(i * 16,  0, 16, 16), QIcon::Active, QIcon::On);
        icon.addPixmap(extentsPixmap.copy(i * 16, 16, 16, 16), QIcon::Active, QIcon::Off);
        icon.addPixmap(extentsPixmap.copy(i * 16, 32, 16, 16), QIcon::Normal, QIcon::Off);
        impl->_buttonIcons.append(icon);
    }
    QPixmap buttonsPixmap(":/Resources/buttons.png");
    for (int i = 0; i < 4; i++)
    {
        QIcon icon;
        icon.addPixmap(buttonsPixmap.copy(i * 16,  0, 16, 16), QIcon::Active, QIcon::On);
        icon.addPixmap(buttonsPixmap.copy(i * 16, 16, 16, 16), QIcon::Active, QIcon::Off);
        icon.addPixmap(buttonsPixmap.copy(i * 16, 32, 16, 16), QIcon::Normal, QIcon::Off);
        impl->_buttonIcons.append(icon);
    }
    
    qApp->installNativeEventFilter(impl.data());
}

FlexManager::~FlexManager()
{

}

FlexManager* FlexManager::instance()
{
    static FlexManager manager; return &manager;
}

FlexWidget* FlexManager::createFlexWidget(Flex::ViewMode viewMode, QWidget* parent, Qt::WindowFlags flags, const QString& flexWidgetName)
{
    FlexWidget* widget = new FlexWidget(viewMode, parent, flags);
    widget->setObjectName(flexWidgetName);
    connect(widget, SIGNAL(destroyed(QObject*)), SLOT(on_flexWidget_destroyed(QObject*)));
    connect(widget, SIGNAL(enterMove(QObject*)), SLOT(on_flexWidget_enterMove(QObject*)));
    connect(widget, SIGNAL(leaveMove(QObject*)), SLOT(on_flexWidget_leaveMove(QObject*)));
    connect(widget, SIGNAL(moving(QObject*)), SLOT(on_flexWidget_moving(QObject*)));
    connect(widget, SIGNAL(destroying(FlexWidget*)), SIGNAL(flexWidgetDestroying(FlexWidget*)));
    widget->installEventFilter(this);
    impl->_flexWidgets.append(widget);
    emit flexWidgetCreated(widget);
    return widget;
}

DockWidget* FlexManager::createDockWidget(Flex::ViewMode viewMode, QWidget* parent, Qt::WindowFlags flags, const QString& dockWidgetName)
{
    DockWidget* widget = new DockWidget(viewMode, parent, flags);
    widget->setObjectName(dockWidgetName);
    connect(widget, SIGNAL(destroyed(QObject*)), SLOT(on_dockWidget_destroyed(QObject*)));
    connect(widget, SIGNAL(enterMove(QObject*)), SLOT(on_dockWidget_enterMove(QObject*)));
    connect(widget, SIGNAL(leaveMove(QObject*)), SLOT(on_dockWidget_leaveMove(QObject*)));
    connect(widget, SIGNAL(moving(QObject*)), SLOT(on_dockWidget_moving(QObject*)));
    connect(widget, SIGNAL(destroying(DockWidget*)), SIGNAL(dockWidgetDestroying(DockWidget*)));
    widget->installEventFilter(this);
    impl->_dockWidgets.append(widget);
    emit dockWidgetCreated(widget);
    return widget;
}

bool FlexManager::hasFlexWidget(const QString& flexWidgetName) const
{
    return std::find_if(impl->_flexWidgets.begin(), impl->_flexWidgets.end(), [&](FlexWidget* flexWidget) { return flexWidget->objectName() == flexWidgetName; }) != impl->_flexWidgets.end();
}

bool FlexManager::hasDockWidget(const QString& dockWidgetName) const
{
    return std::find_if(impl->_dockWidgets.begin(), impl->_dockWidgets.end(), [&](DockWidget* dockWidget) { return dockWidget->objectName() == dockWidgetName; }) != impl->_dockWidgets.end();
}

FlexWidget* FlexManager::flexWidget(const QString& flexWidgetName) const
{
    auto iter = std::find_if(impl->_flexWidgets.begin(), impl->_flexWidgets.end(), [&](FlexWidget* flexWidget) { return flexWidget->objectName() == flexWidgetName; });
    return iter != impl->_flexWidgets.end() ? *iter : nullptr;
}

DockWidget* FlexManager::dockWidget(const QString& dockWidgetName) const
{
    auto iter = std::find_if(impl->_dockWidgets.begin(), impl->_dockWidgets.end(), [&](DockWidget* dockWidget) { return dockWidget->objectName() == dockWidgetName; });
    return iter != impl->_dockWidgets.end() ? *iter : nullptr;
}

int FlexManager::flexWidgetCount() const
{
    return (int)impl->_flexWidgets.size();
}

int FlexManager::dockWidgetCount() const
{
    return (int)impl->_dockWidgets.size();
}

FlexWidget* FlexManager::flexWidgetAt(int index) const
{
    Q_ASSERT(index >= 0 && index < impl->_flexWidgets.size());
    return impl->_flexWidgets[index];
}

DockWidget* FlexManager::dockWidgetAt(int index) const
{
    Q_ASSERT(index >= 0 && index < impl->_dockWidgets.size());
    return impl->_dockWidgets[index];
}

QIcon FlexManager::icon(Flex::Button button)
{
    return impl->_buttonIcons[button];
}

void FlexManager::close()
{
    for (auto iter = impl->_dockWidgets.begin(); iter != impl->_dockWidgets.end(); ++iter)
    {
        if ((*iter)->isWindow())
        {
            (*iter)->deleteLater();
        }
    }
    for (auto iter = impl->_flexWidgets.begin(); iter != impl->_flexWidgets.end(); ++iter)
    {
        if ((*iter)->isWindow())
        {
            (*iter)->deleteLater();
        }
    }
}

bool FlexManager::load(const QByteArray& content, const QMap<QString, QWidget*>& parents)
{
    close();

    QJsonObject object = QJsonDocument::fromJson(content).object();

    QJsonArray flexWidgetObjects = object["flexWidgets"].toArray();

    for (int i = 0; i < flexWidgetObjects.size(); ++i)
    {
        QJsonObject flexWidgetObject = flexWidgetObjects[i].toObject();

        Flex::ViewMode viewMode = (Flex::ViewMode)flexWidgetObject["viewMode"].toInt();
        QWidget* parent = parents.value(flexWidgetObject["parent"].toString(), nullptr);
        Qt::WindowFlags flags = (Qt::WindowFlags)flexWidgetObject["windowFlags"].toInt();
        QString flexWidgetName = flexWidgetObject["flexWidgetName"].toString();

        FlexWidget* flexWidget = createFlexWidget(viewMode, parent, Flex::widgetFlags(), flexWidgetName);

        flexWidget->load(flexWidgetObject);
    }

    return true;
}

QByteArray FlexManager::save()
{
    QJsonObject object;

    QJsonArray flexWidgetObjects;

    for (int i = 0; i < impl->_flexWidgets.size(); ++i)
    {
        FlexWidget* flexWidget = impl->_flexWidgets[i];

        QJsonObject flexWidgetObject;

        flexWidgetObject["viewMode"] = (int)flexWidget->viewMode();
        flexWidgetObject["parent"] = flexWidget->parentWidget() ? flexWidget->parentWidget()->objectName() : "";
        flexWidgetObject["windowFlags"] = (int)Flex::windowFlags(flexWidget->viewMode());
        flexWidgetObject["flexWidgetName"] = flexWidget->objectName();

        flexWidget->save(flexWidgetObject);

        flexWidgetObjects.append(flexWidgetObject);
    }

    object["flexWidgets"] = flexWidgetObjects;

    return QJsonDocument(object).toJson();
}

bool FlexManager::eventFilter(QObject* obj, QEvent* evt)
{
#ifndef Q_OS_WIN
    if (evt->type() == QEvent::WindowActivate)
    {
        FlexWidget* flexWidget;
        if ((flexWidget = qobject_cast<FlexWidget*>(obj)) != nullptr)
        {
            if (impl->_flexWidgets.front() != flexWidget && impl->_flexWidgets.removeOne(flexWidget))
            {
                impl->_flexWidgets.push_front(flexWidget);
            }
            return false;
        }
        DockWidget* dockWidget;
        if ((dockWidget = qobject_cast<DockWidget*>(obj)) != nullptr)
        {
            if (impl->_dockWidgets.front() != dockWidget && impl->_dockWidgets.removeOne(dockWidget))
            {
                impl->_dockWidgets.push_front(dockWidget);
            }
            return false;
        }
    }
#endif
    return false;
}

void FlexManager::on_dockWidget_destroyed(QObject* widget)
{
    impl->_dockWidgets.removeOne(static_cast<DockWidget*>(widget));
}

void FlexManager::on_flexWidget_destroyed(QObject* widget)
{
    impl->_flexWidgets.removeOne(static_cast<FlexWidget*>(widget));
}

void FlexManager::on_flexWidget_guiderShow(FlexWidget* flexWidget, QWidget* widget)
{
    Q_ASSERT(widget != nullptr); 
#ifdef Q_OS_WIN
    SetWindowPos(reinterpret_cast<HWND>(flexWidget->window()->effectiveWinId()), reinterpret_cast<HWND>(widget->effectiveWinId()), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
#else
    flexWidget->window()->raise();
    if (impl->_flexWidgets.size() > 1 && impl->_flexWidgets[1] != flexWidget && impl->_flexWidgets.removeOne(flexWidget))
    {
        impl->_flexWidgets.insert(1, flexWidget);
    }
    widget->raise();
#endif
    flexWidget->showGuider(widget);
}

void FlexManager::on_flexWidget_guiderHide(FlexWidget* flexWidget, QWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    flexWidget->hideGuider(widget);
}

void FlexManager::on_flexWidget_guiderHover(FlexWidget* flexWidget, QWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    flexWidget->hoverGuider(widget);
}

void FlexManager::on_flexWidget_guiderDrop(FlexWidget* flexWidget, DockWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    if (flexWidget->dropGuider(widget))
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(flexWidget->window()->effectiveWinId()), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | 1800);
#else
        flexWidget->window()->raise();
#endif
    }
    else
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(widget->effectiveWinId()), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | 1800);
#else
        widget->raise();
#endif
    }
}

void FlexManager::on_flexWidget_guiderDrop(FlexWidget* flexWidget, FlexWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    if (flexWidget->dropGuider(widget))
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(flexWidget->window()->effectiveWinId()), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | 1800);
#else
        flexWidget->window()->raise();
#endif
    }
    else
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(widget->effectiveWinId()), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | 1800);
#else
        widget->raise();
#endif
    }
}

void FlexManager::on_flexWidget_enterMove(QObject*)
{
    impl->_ready = true;
}

void FlexManager::on_flexWidget_leaveMove(QObject* object)
{
    if (!impl->_ready)
    {
        return;
    }

    auto widget = static_cast<FlexWidget*>(object);

    auto pos = QCursor::pos();

#ifdef Q_OS_WIN
    auto top = topLevelWindowAt(widget, pos);
#endif

    auto has = false;

    impl->_ready = false;

    for (auto iter = impl->_flexWidgets.begin(); iter != impl->_flexWidgets.end(); ++iter)
    {
        auto flexWidget = *iter;

        if (flexWidget == widget)
        {
            continue;
        }

        if (flexWidget->window()->isMinimized())
        {
            continue;
        }

#ifdef Q_OS_WIN
        if (!has && flexWidget->window()->effectiveWinId() == top && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#else
        if (!has && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#endif
        {
            if (flexWidget->isGuiderExists())
            {
                if (flexWidget->isGuiderVisible())
                {
                    emit guiderDrop(flexWidget, widget);
                }
                else
                {
                    emit guiderHide(flexWidget, widget);
                }
            }
            has = true;
        }
        else
        {
            if (flexWidget->isGuiderExists())
            {
                emit guiderHide(flexWidget, widget); break;
            }
        }
    }
}

void FlexManager::on_flexWidget_moving(QObject* object)
{
    if (!impl->_ready)
    {
        return;
    }

    auto widget = static_cast<FlexWidget*>(object);

    auto pos = QCursor::pos();

#ifdef Q_OS_WIN
    auto top = topLevelWindowAt(widget, pos);
#endif

    auto has = false;

    for (auto iter = impl->_flexWidgets.begin(); iter != impl->_flexWidgets.end(); ++iter)
    {
        auto flexWidget = *iter;

        if (flexWidget == widget)
        {
            continue;
        }

        if (flexWidget->window()->isMinimized())
        {
            continue;
        }

#ifdef Q_OS_WIN
        if (!has && flexWidget->window()->effectiveWinId() == top && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#else
        if (!has && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#endif
        {
            if (flexWidget->isGuiderExists())
            {
                if (flexWidget->isGuiderVisible())
                {
                    emit guiderHover(flexWidget, widget);
                }
            }
            else
            {
                emit guiderShow(flexWidget, widget);
            }

            has = true;
        }
        else
        {
            if (flexWidget->isGuiderExists())
            {
                emit guiderHide(flexWidget, widget);
            }
        }
    }
}

void FlexManager::on_dockWidget_enterMove(QObject*)
{
    impl->_ready = true;
}

void FlexManager::on_dockWidget_leaveMove(QObject* object)
{
    if (!impl->_ready)
    {
        return;
    }

    auto widget = static_cast<DockWidget*>(object);

    auto pos = QCursor::pos();

#ifdef Q_OS_WIN
    auto top = topLevelWindowAt(widget, pos);
#endif

    auto has = false;

    impl->_ready = false;

    for (auto iter = impl->_flexWidgets.begin(); iter != impl->_flexWidgets.end(); ++iter)
    {
        auto flexWidget = *iter;

        if (flexWidget->window()->isMinimized())
        {
            continue;
        }

#ifdef Q_OS_WIN
        if (!has && flexWidget->window()->effectiveWinId() == top && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#else
        if (!has && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#endif
        {
            if (flexWidget->isGuiderExists())
            {
                if (flexWidget->isGuiderVisible())
                {
                    emit guiderDrop(flexWidget, widget);
                }
                else
                {
                    emit guiderHide(flexWidget, widget);
                }
            }
            has = true;
        }
        else
        {
            if (flexWidget->isGuiderExists())
            {
                emit guiderHide(flexWidget, widget); break;
            }
        }
    }
}

void FlexManager::on_dockWidget_moving(QObject* object)
{
    if (!impl->_ready)
    {
        return;
    }

    auto widget = static_cast<DockWidget*>(object);

    auto pos = QCursor::pos();

#ifdef Q_OS_WIN
    auto top = topLevelWindowAt(widget, pos);
#endif

    auto has = false;

    for (auto iter = impl->_flexWidgets.begin(); iter != impl->_flexWidgets.end(); ++iter)
    {
        auto flexWidget = *iter;

        if (flexWidget->window()->isMinimized())
        {
            continue;
        }

#ifdef Q_OS_WIN
        if (!has && flexWidget->window()->effectiveWinId() == top && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#else
        if (!has && flexWidget->isDockAllowed(widget, flexWidget->mapFromGlobal(pos)))
#endif
        {
            if (flexWidget->isGuiderExists())
            {
                if (flexWidget->isGuiderVisible())
                {
                    emit guiderHover(flexWidget, widget);
                }
            }
            else
            {
                emit guiderShow(flexWidget, widget);
            }
            has = true;
        }
        else
        {
            if (flexWidget->isGuiderExists())
            {
                emit guiderHide(flexWidget, widget);
            }
        }
    }
}

void FlexManager::on_app_focusChanged(QWidget* old, QWidget* now)
{
    FlexWidget* flexWidget = nullptr;

    auto oldDockSite = getDockSite(old);
    auto nowDockSite = getDockSite(now);

    if (nowDockSite != nullptr && oldDockSite != nowDockSite)
    {
        if (oldDockSite)
        {
            oldDockSite->setActive(false);
        }

        if (!nowDockSite->isActive())
        {
            if ((flexWidget = nowDockSite->flexWidget()) != nullptr)
            {
                flexWidget->setCurrent(nowDockSite);
            }
            if (nowDockSite->isActive())
            {
                emit dockSiteActivated(nowDockSite);
            }
        }
    }
}

QWidget* Flex::window()
{
    return qobject_cast<QWidget*>(qApp->property("window").value<QObject*>());
}

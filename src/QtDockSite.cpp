#include "QtDockSite.h"
#include "QtDockWidget.h"
#include "QtFlexWidget.h"
#include "QtFlexHelper.h"
#include "QtFlexManager.h"
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QSplitter>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

class DockSiteTabBar : public QTabBar
{
public:
    DockSiteTabBar(QWidget* parent) : QTabBar(parent)
    {
        setMovable(true);
        setExpanding(false);
        setProperty("Flex", true);
        setProperty("active", false);
    }
public:
    QSize tabSizeHint(int index) const
    {
        return QSize(QTabBar::tabSizeHint(index).width(), shape() == QTabBar::RoundedNorth ? 22 : 21);
    }
public:
    void closeTab(int index)
    {
        emit tabCloseRequested(index);
    }
};

class DockSiteTabMdi : public QStackedWidget
{
public:
    DockSiteTabMdi(QWidget* parent) : QStackedWidget(parent)
    {
        setProperty("Flex", true);
        setProperty("active", false);
    }
protected:
    void paintEvent(QPaintEvent*)
    {
    }
};

class DockSiteImpl
{
public:
    DockSiteImpl() : _active(false), _viewMode(Flex::HybridView), _dockMode(Flex::DockInNoneArea), _startDrag(false), _startPull(false)
    {
    }
public:
    void update(DockSite* self, Flex::DockMode dockMode);
    void update(DockSite* self, DockWidget* widget, int method);
    void adjust(DockSite* self, DockWidget* widget);

public:
    bool isTitleBarVisible(DockSite* self, QRect* rect = nullptr) const;

public:
    bool _active;
    Flex::ViewMode _viewMode;
    Flex::DockMode _dockMode;
    QVBoxLayout* _tabBarLayout;
    QVBoxLayout* _tabMdiLayout;
    DockSiteTabBar* _tabBar;
    DockSiteTabMdi* _tabMdi;
    FlexHelper* _helper = nullptr;
    bool _persistent = false;
    bool _startDrag;
    bool _startPull;
    QPoint _startPoint;
    int _titleBarHeight;
};

void DockSiteImpl::update(DockSite* self, Flex::DockMode dockMode)
{
    if (_dockMode != dockMode)
    {
        _dockMode = dockMode;
    }

    auto tempWidget = self->flexWidget();

    if (isTitleBarVisible(self))
    {
        if (tempWidget) tempWidget->setWindowTitle(Flex::window()->windowTitle());
        self->setContentsMargins(0, _titleBarHeight, 0, 0);
        if (!_helper)
        {
            _helper = new FlexHelper(self);
            _helper->setWindowInfo(_titleBarHeight, Qt::Window);
            _helper->button(Flex::Minimize)->hide();
            _helper->button(Flex::Maximize)->hide();
            self->connect(_helper, SIGNAL(clicked(Flex::Button, bool*)), SLOT(on_titleBar_buttonClicked(Flex::Button, bool*)));
        }
        if (_tabBar->isHidden())
        {
            _tabMdiLayout->setContentsMargins(1, 0, 1, 1);
        }
        else
        {
            _tabMdiLayout->setContentsMargins(1, 0, 1, 0);
        }
    }
    else
    {
        if (tempWidget) tempWidget->setWindowTitle(_tabBar->tabText(_tabBar->currentIndex()));
        self->setContentsMargins(0, 0, 0, 0);
        if (_helper)
        {
            _helper->deleteLater(); _helper = nullptr;
        }
        if (_tabBar->isHidden())
        {
            _tabMdiLayout->setContentsMargins(0, 0, 0, 0);
        }
        else
        {
            _tabMdiLayout->setContentsMargins(1, 0, 1, 1);
        }
    }

    switch (_dockMode)
    {
    case Flex::DockInMainArea:
        if (_helper)
        {
            _helper->change(Flex::DockShow, Flex::AutoHide);
        }
        break;
    case Flex::DockInSideArea:
        if (_helper)
        {
            _helper->change(Flex::AutoHide, Flex::DockShow);
        }
        break;
    }

    if (tempWidget)
    {
        tempWidget->update();
    }

    qApp->sendEvent(self, new QResizeEvent(self->size(), QSize()));
}

void DockSiteImpl::adjust(DockSite* self, DockWidget* widget)
{
    auto arranger = qobject_cast<QVBoxLayout*>(self->layout());

    QPalette tabPalette = _tabBar->palette();

    if (_persistent || !widget || widget->viewMode() == Flex::FileView || widget->viewMode() == Flex::FilePagesView)
    {
        tabPalette.setColor(QPalette::Active, QPalette::Highlight, QColor("#FFF29D"));
        tabPalette.setColor(QPalette::Inactive, QPalette::Highlight, QColor("#4D6082"));
        tabPalette.setColor(QPalette::Active, _tabBar->backgroundRole(), QColor("#5B7199"));
        tabPalette.setColor(QPalette::Inactive, _tabBar->backgroundRole(), QColor("#364E6F"));
        tabPalette.setColor(QPalette::Active, _tabBar->foregroundRole(), QColor("#000000"));
        tabPalette.setColor(QPalette::Inactive, _tabBar->foregroundRole(), QColor("#FFFFFF"));
        arranger->setContentsMargins(0, 0, 0, 0);
        arranger->addLayout(_tabBarLayout, 0);
        arranger->addLayout(_tabMdiLayout, 1);
        _tabBar->setTabsClosable(true);
        _tabBar->setShape(QTabBar::RoundedNorth);
        _tabBar->show();
    }
    else
    {
        tabPalette.setColor(QPalette::Active, QPalette::Highlight, QColor("#FFFFFF"));
        tabPalette.setColor(QPalette::Inactive, QPalette::Highlight, QColor("#FFFFFF"));
        tabPalette.setColor(QPalette::Active, _tabBar->backgroundRole(), QColor("#5B7199"));
        tabPalette.setColor(QPalette::Inactive, _tabBar->backgroundRole(), QColor("#4D6082"));
        tabPalette.setColor(QPalette::Active, _tabBar->foregroundRole(), QColor("#000000"));
        tabPalette.setColor(QPalette::Inactive, _tabBar->foregroundRole(), QColor("#FFFFFF"));
        arranger->setContentsMargins(0, 0, 0, 0);
        arranger->addLayout(_tabMdiLayout, 1);
        arranger->addLayout(_tabBarLayout, 0);
        _tabBar->setTabsClosable(false);
        _tabBar->setShape(QTabBar::RoundedSouth);
        _tabBar->hide();
    }

    _tabBar->setPalette(tabPalette);
}

void DockSiteImpl::update(DockSite* self, DockWidget* widget, int method)
{
    if (method == 0)
    {
        auto dockViewMode = widget ? widget->viewMode() : Flex::HybridView;

        if (_persistent)
        {
            _viewMode = Flex::HybridView;
        }
        else if (_tabBar->count() == 1)
        {
            _viewMode = dockViewMode;
        }
        else if (_viewMode == Flex::HybridView)
        {
            _viewMode = Flex::HybridView;
        }
        else if ((_viewMode == Flex::ToolView || _viewMode == Flex::ToolPagesView) && (dockViewMode == Flex::ToolView || dockViewMode == Flex::ToolPagesView))
        {
            _viewMode = Flex::ToolPagesView;
        }
        else if ((_viewMode == Flex::FileView || _viewMode == Flex::FilePagesView) && (dockViewMode == Flex::FileView || dockViewMode == Flex::FilePagesView))
        {
            _viewMode = Flex::FilePagesView;
        }
        else
        {
            _viewMode = Flex::HybridView;
        }
    }
    else
    {
        auto dockWidgets = self->widgets();

        if (_persistent)
        {
            _viewMode = Flex::HybridView;
        }
        else if (dockWidgets.size() == 1)
        {
            _viewMode = dockWidgets[0]->viewMode();
        }
        else if (!dockWidgets.empty())
        {
            if (_viewMode != Flex::HybridView)
            {
            }
            else if (std::all_of(dockWidgets.begin(), dockWidgets.end(), [&](DockWidget* dockWidget) { return dockWidget->viewMode() == Flex::ToolView; }))
            {
                _viewMode = Flex::ToolPagesView;
            }
            else if (std::all_of(dockWidgets.begin(), dockWidgets.end(), [&](DockWidget* dockWidget) { return dockWidget->viewMode() == Flex::FileView; }))
            {
                _viewMode = Flex::FilePagesView;
            }
        }
    }
}

bool DockSiteImpl::isTitleBarVisible(DockSite* self, QRect* rect) const
{
    auto tempWidget = self->flexWidget();

    if (_persistent || !tempWidget || (tempWidget->isFloating() && (tempWidget->viewMode() == Flex::ToolView || tempWidget->viewMode() == Flex::ToolPagesView)))
    {
        if (rect) *rect = QRect(); return false;
    }

    if (_viewMode == Flex::ToolView || _viewMode == Flex::ToolPagesView)
    {
        if (rect) *rect = QRect(0, 0, self->width(), _titleBarHeight); return true;
    }
    else
    {
        if (rect) *rect = QRect(); return false;
    }
}

DockSite::DockSite(DockWidget* widget, QSize baseSize, QWidget* parent) : QWidget(parent), impl(new DockSiteImpl)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::StrongFocus);
    setProperty("Flex", true);
    setBaseSize(baseSize);

    auto arranger = new QVBoxLayout(this);

    arranger->setObjectName("_flex_siteLayout");

    impl->_tabBarLayout = new QVBoxLayout();
    impl->_tabBarLayout->setObjectName("_flex_tabBarLayout");
    impl->_tabBarLayout->setSpacing(0);
    impl->_tabMdiLayout = new QVBoxLayout();
    impl->_tabMdiLayout->setObjectName("_flex_tabMdiLayout");
    impl->_tabMdiLayout->setSpacing(0);

    impl->_titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, this) - 4;

    impl->_tabBar = new DockSiteTabBar(this);
    impl->_tabBar->setObjectName("_flex_tabBar");
    impl->_tabMdi = new DockSiteTabMdi(this);
    impl->_tabMdi->setObjectName("_flex_tabMdi");

    impl->_tabBarLayout->addWidget(impl->_tabBar);
    impl->_tabMdiLayout->addWidget(impl->_tabMdi);

    arranger->setSpacing(0);

    if (widget)
    {
        setWindowTitle(widget->windowTitle());
    }

    impl->_persistent = (widget == nullptr);

    impl->adjust(this, widget);

    if (widget)
    {
        impl->_tabMdi->addWidget(widget);
    }

    connect(impl->_tabBar, SIGNAL(currentChanged(int)), SLOT(on_tabBar_currentChanged(int)));
    connect(impl->_tabBar, SIGNAL(tabCloseRequested(int)), SLOT(on_tabBar_tabCloseRequested(int)));
    connect(impl->_tabMdi, SIGNAL(currentChanged(int)), SLOT(on_tabMdi_currentChanged(int)));
    connect(impl->_tabMdi, SIGNAL(widgetRemoved(int)), SLOT(on_tabMdi_widgetRemoved(int)));

    if (widget)
    {
        impl->_tabBar->addTab(widget->windowTitle());
    }

    impl->_tabBar->installEventFilter(this);

    impl->update(this, widget, 0);

    impl->update(this, Flex::DockInNoneArea);

    if (widget)
    {
        resize(widget->size());
    }
}

DockSite::~DockSite()
{

}

Flex::ViewMode DockSite::viewMode() const
{
    return impl->_viewMode;
}

Flex::DockMode DockSite::dockMode() const
{
    return impl->_dockMode;
}

void DockSite::setDockMode(Flex::DockMode dockMode)
{
    if (impl->_dockMode != dockMode)
    {
        impl->update(this, dockMode);
    }
}

Flex::Features DockSite::features() const
{
    return impl->_tabMdi->count() > 0 ? qobject_cast<DockWidget*>(impl->_tabMdi->widget(0))->siteFeatures() : (Flex::AllowDockAsNorthTabPage | Flex::AllowDockAsSouthTabPage);
}

bool DockSite::addWidget(DockWidget* widget)
{
    Q_ASSERT(widget != nullptr);
    
    if (impl->_tabBar->addTab(widget->windowTitle()) >= 0)
    {
        impl->_tabMdi->blockSignals(true);
        impl->_tabMdi->addWidget(widget);
        impl->_tabMdi->blockSignals(false);
    }

    if (impl->_tabBar->count() > 1 || impl->_persistent)
    {
        impl->_tabBar->show(); impl->_tabBar->setCurrentIndex(impl->_tabBar->count() - 1);
    }
    else
    {
        impl->adjust(this, widget);
    }

    impl->update(this, widget, 0);

    impl->update(this, impl->_dockMode);

    return true;
}

FlexWidget* DockSite::flexWidget() const
{
    QWidget* tempWidget = parentWidget();
    while (tempWidget && !qobject_cast<FlexWidget*>(tempWidget))
    {
        tempWidget = tempWidget->parentWidget();
    }
    return qobject_cast<FlexWidget*>(tempWidget);
}

int DockSite::currentIndex() const
{
    return impl->_tabMdi->currentIndex();
}

int DockSite::count() const
{
    return impl->_tabMdi->count();
}

DockWidget* DockSite::widget(int index) const
{
    return qobject_cast<DockWidget*>(impl->_tabMdi->widget(index));
}

QList<DockWidget*> DockSite::widgets() const
{
    QList<DockWidget*> dockWidgets;
    for (int i = 0; i < impl->_tabMdi->count(); ++i)
    {
        dockWidgets.append(static_cast<DockWidget*>(impl->_tabMdi->widget(i)));
    }
    return dockWidgets;
}

void DockSite::removeWidgetAt(int index)
{
    impl->_tabBar->closeTab(index);
}

bool DockSite::isActive() const
{
    return impl->_active;
}

void DockSite::setActive(bool active)
{
    if (impl->_active != active)
    {
        impl->_active = active;
        impl->_tabBar->setProperty("active", active);
        impl->_tabBar->update();
        impl->_tabMdi->setProperty("active", active);
        impl->_tabMdi->update();
        update();
    }
}

void DockSite::activate()
{
    setFocus();
}

bool DockSite::event(QEvent* evt)
{
    if (evt->type() == Flex::Update)
    {
        impl->update(this, Flex::DockInMainArea);
    }
    else if (evt->type() == QEvent::ParentAboutToChange)
    {
        auto tempWidget = flexWidget();

        if (tempWidget)
        {
            if (!tempWidget->isAdjusting() && !impl->_persistent)
            {
                tempWidget->removeDockSite(this);
            }
        }
    }

    return QWidget::event(evt);
}

bool DockSite::eventFilter(QObject* obj, QEvent* evt)
{
    if (obj == impl->_tabBar)
    {
        switch (evt->type())
        {
        case QEvent::MouseButtonPress:
        {
            QMouseEvent* mouse = static_cast<QMouseEvent*>(evt);

            if (mouse->button() == Qt::LeftButton && impl->_tabBar->tabAt(mouse->pos()) != -1)
            {
                auto tempWidget = flexWidget();

                if (!tempWidget->isFloating() || impl->_tabBar->count() > 1)
                {
                    impl->_startPull = true;
                    impl->_startPoint = mouse->pos();
                    impl->_tabBar->grabMouse();
                }
            }

            break;
        }
        case QEvent::MouseButtonRelease:
        {
            impl->_tabBar->releaseMouse();
            impl->_startPull = false;
            break;
        }
        case QEvent::MouseMove:
        {
            if (impl->_startPull)
            {
                QMouseEvent* mouse = static_cast<QMouseEvent*>(evt);

                int offset = abs(mouse->pos().y() - impl->_startPoint.y());

                if (offset < QApplication::startDragDistance() * 5)
                {
                    return QWidget::eventFilter(obj, evt);
                }

                QApplication::sendEvent(impl->_tabBar, &QMouseEvent(QEvent::MouseButtonRelease, mouse->pos(), Qt::LeftButton, mouse->buttons(), mouse->modifiers()));

                auto dimension = size();

                auto flexWidget = FlexManager::instance()->createFlexWidget(viewMode(), Flex::parent(viewMode()), Flex::windowFlags());

                int current = impl->_tabBar->currentIndex();

                DockWidget* dockWidget = qobject_cast<DockWidget*>(impl->_tabMdi->widget(current));

                flexWidget->addDockSite(new DockSite(dockWidget), Flex::M, -1);

                auto pos = QCursor::pos();
                auto tl = flexWidget->geometry().topLeft() - flexWidget->frameGeometry().topLeft();
                auto br = flexWidget->frameGeometry().bottomRight() - flexWidget->geometry().bottomRight();
                flexWidget->setGeometry(QRect(pos + tl - impl->_startPoint, dimension - QSize(tl.x(), tl.y()) - QSize(br.x(), br.y())));
                flexWidget->show();

                impl->update(this, dockWidget, 1);

                QApplication::sendPostedEvents();

#ifdef Q_OS_WIN
                SendMessage((HWND)effectiveWinId(), WM_LBUTTONUP, 0, MAKELONG(pos.x(), pos.y()));
                SendMessage((HWND)flexWidget->effectiveWinId(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELONG(pos.x(), pos.y()));
#endif
                return true;
            }
            break;
        }
        }
    }
    return QWidget::eventFilter(obj, evt);
}

void DockSite::paintEvent(QPaintEvent*)
{
    if (impl->_tabBar->count() == 0)
    {
        return;
    }

    QStylePainter painter(this);

    QRect titleBarRect;

    if (impl->isTitleBarVisible(this, &titleBarRect))
    {
        if (impl->_helper)
        {
            impl->_helper->buttons()->show();
            impl->_helper->extents()->show();
        }

        QStyleOptionTitleBar titleOption;
        titleOption.init(this);
        titleOption.rect = titleBarRect;
        titleOption.text = impl->_tabBar->tabText(impl->_tabBar->currentIndex());
        titleOption.subControls = QStyle::SC_TitleBarLabel;

        bool active = impl->_active;

        painter.fillRect(titleBarRect, QColor(active ? "#fff29d" : "#4D6082"));

        QRect ir = style()->subControlRect(QStyle::CC_TitleBar, &titleOption, QStyle::SC_TitleBarLabel, this);

        painter.setPen(active ? Qt::black : Qt::white);

        painter.drawText(ir.x() + 2, ir.y(), ir.width() - 2, ir.height(), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, titleOption.text);
    }
    else
    {
        if (impl->_helper)
        {
            impl->_helper->buttons()->hide();
            impl->_helper->extents()->hide();
        }
    }

    int tabBarHeight = impl->_tabBar->height();

    QStyleOptionFrame frameOption;

    frameOption.initFrom(this);

    if (impl->_tabBar->isVisible())
    {
        if (impl->_tabBar->shape() == QTabBar::RoundedNorth)
        {
            frameOption.rect.adjust(0, +tabBarHeight, 0, 0);
        }
        else
        {
            frameOption.rect.adjust(0, 0, 0, -tabBarHeight);
        }
    }

    painter.drawPrimitive(QStyle::PE_Frame, frameOption);
}

void DockSite::closeEvent(QCloseEvent* evt)
{
    setParent(nullptr); evt->accept();
}

void DockSite::mousePressEvent(QMouseEvent* evt)
{
    QRect titleBarRect;

    if (evt->button() == Qt::LeftButton && impl->isTitleBarVisible(this, &titleBarRect) && !isTopLevel())
    {
        if (titleBarRect.contains(evt->pos()))
        {
            impl->_startPoint = evt->pos(); 

            auto tempWidget = flexWidget();

            if (!tempWidget->isFloating() || tempWidget->count() > 1)
            {
                impl->_startDrag = true;
            }
        }
    }
}

void DockSite::mouseMoveEvent(QMouseEvent* evt)
{
    QPoint offset = evt->pos() - impl->_startPoint;

    if (impl->_startDrag)
    {
        if (offset.manhattanLength() < QApplication::startDragDistance())
        {
            return;
        }

        auto dimension = size();

        auto flexWidget = FlexManager::instance()->createFlexWidget(viewMode(), Flex::parent(viewMode()), Flex::windowFlags());
        
        flexWidget->addDockSite(this, Flex::M, -1);

        auto pos = QCursor::pos();
        auto tl = flexWidget->geometry().topLeft() - flexWidget->frameGeometry().topLeft();
        auto br = flexWidget->frameGeometry().bottomRight() - flexWidget->geometry().bottomRight();
        flexWidget->setGeometry(QRect(pos + tl - impl->_startPoint, dimension - QSize(tl.x(), tl.y()) - QSize(br.x(), br.y())));
        flexWidget->show();

        QApplication::sendPostedEvents();

#ifdef Q_OS_WIN
        SendMessage((HWND)effectiveWinId(), WM_LBUTTONUP, 0, MAKELONG(pos.x(), pos.y()));
        SendMessage((HWND)flexWidget->effectiveWinId(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELONG(pos.x(), pos.y()));
#endif

        impl->_startDrag = false;
    }
}

void DockSite::mouseReleaseEvent(QMouseEvent*)
{
    impl->_startDrag = false;
}

void DockSite::mouseDoubleClickEvent(QMouseEvent* evt)
{
    QRect titleBarRect;

    if (evt->button() == Qt::LeftButton && impl->isTitleBarVisible(this, &titleBarRect) && titleBarRect.contains(evt->pos()))
    {
        auto tempWidget = flexWidget();

        if (!tempWidget->isFloating() || tempWidget->siteContainer()->count() > 1)
        {
            auto dimension = size();
            auto flexWidget = FlexManager::instance()->createFlexWidget(viewMode(), Flex::parent(viewMode()), Flex::windowFlags());
            flexWidget->addDockSite(this, Flex::M, -1);
            auto pos = evt->globalPos();
            auto tl = flexWidget->geometry().topLeft() - flexWidget->frameGeometry().topLeft();
            auto br = flexWidget->frameGeometry().bottomRight() - flexWidget->geometry().bottomRight();
            flexWidget->setGeometry(QRect(pos + tl - impl->_startPoint, dimension - QSize(tl.x(), tl.y()) - QSize(br.x(), br.y())));
            flexWidget->show();
        }
    }
}

void DockSite::on_tabBar_currentChanged(int index)
{
    auto tempWidget = flexWidget();

    if (impl->_tabBar->count() == 1 && impl->_tabBar->shape() == QTabBar::RoundedSouth)
    {
        impl->_tabBar->hide();
    }
    else if (impl->_tabBar->count() == 0)
    {
        impl->_tabBar->hide();
    }

    impl->_tabMdi->blockSignals(true);
    impl->_tabMdi->setCurrentIndex(index); 
    impl->_tabMdi->blockSignals(false);

    if (tempWidget)
    {
        if (impl->isTitleBarVisible(this))
        {
            tempWidget->setWindowTitle(Flex::window()->windowTitle());
        }
        else
        {
            tempWidget->setWindowTitle(impl->_tabBar->tabText(index));
        }

        tempWidget->update();
    }

    update();
}

void DockSite::on_tabBar_tabCloseRequested(int index)
{
    auto widget = static_cast<DockWidget*>(impl->_tabMdi->widget(index));

    impl->_tabBar->removeTab(index);

    impl->_tabMdi->blockSignals(true);
    impl->_tabMdi->removeWidget(widget);
    impl->_tabMdi->blockSignals(false);

    widget->setParent(nullptr); 

    widget->close();

    auto tempWidget = flexWidget();

    if (impl->_tabBar->count() == 0)
    {
        if (!impl->_persistent)
        {
            tempWidget->removeDockSite(this);
        }
    }

    if (tempWidget->isFloating() && tempWidget->count() == 0 && impl->_tabBar->count() == 0)
    {
        tempWidget->close();
    }

    impl->update(this, widget, 1);

    update();
}

void DockSite::on_tabMdi_currentChanged(int index)
{
    auto tempWidget = flexWidget();

    if (impl->_tabBar->count() == 1 && impl->_tabBar->shape() == QTabBar::RoundedSouth)
    {
        impl->_tabBar->hide();
    }
    else if (impl->_tabBar->count() == 0)
    {
        impl->_tabBar->hide();
    }

    impl->_tabBar->blockSignals(true);
    impl->_tabBar->setCurrentIndex(index);
    impl->_tabBar->blockSignals(false);

    if (tempWidget)
    {
        if (impl->isTitleBarVisible(this))
        {
            tempWidget->setWindowTitle(Flex::window()->windowTitle());
        }
        else
        {
            tempWidget->setWindowTitle(impl->_tabBar->tabText(index));
        }

        tempWidget->update();
    }

    update();
}

void DockSite::on_tabMdi_widgetRemoved(int index)
{
    impl->_tabBar->blockSignals(true);
    impl->_tabBar->removeTab(index);
    impl->_tabBar->blockSignals(false);

    auto tempWidget = flexWidget();

    if (impl->_tabBar->count() == 0)
    {
        if (!impl->_persistent)
        {
            tempWidget->removeDockSite(this);
        }
    }

    if (tempWidget->isFloating() && tempWidget->count() == 0 && impl->_tabBar->count() == 0)
    {
        tempWidget->close();
    }

    impl->update(this, nullptr, 1);

    update();
}

void DockSite::on_titleBar_buttonClicked(Flex::Button button, bool *accepted)
{
    switch (button)
    {
    case Flex::AutoHide:
        if (impl->_dockMode == Flex::DockInMainArea)
        {
            flexWidget()->makeSiteAutoHide(this);
        }
        break;
    case Flex::DockShow:
        if (impl->_dockMode == Flex::DockInSideArea)
        {
            flexWidget()->makeSiteDockShow(this);
        }
        break;
    case Flex::DockPull:
        flexWidget()->showSiteDockPull(this);
        break;
    }

    *accepted = true;

    if (button != Flex::Close || (impl->_viewMode != Flex::ToolView && impl->_viewMode != Flex::ToolPagesView))
    {
        return;
    }

    removeWidgetAt(currentIndex());
}

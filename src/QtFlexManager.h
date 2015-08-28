#ifndef QTFLEXMANAGER_H
#define QTFLEXMANAGER_H

#include <QtCore/QScopedPointer>
#include <QtWidgets/QSplitter>

class QTabBar;
class QSplitter;
class QToolButton;
class QStackedWidget;

class DockSide;
class DockSite;
class DockGuider;
class DockWidget;
class FlexWidget;

#ifndef QT_FLEX_API
#  ifdef QT_FLEX_BUILD
#    define QT_FLEX_API Q_DECL_EXPORT
#  else
#    define QT_FLEX_API Q_DECL_IMPORT
#  endif
#endif

class QT_FLEX_API Flex
{
    Q_GADGET
    Q_ENUMS(DockArea)
    Q_ENUMS(DockMode)
    Q_ENUMS(ViewMode)
public:
    enum DockArea
    {
        None = -1,
        M,
        L0,
        T0,
        R0,
        B0,
        L1,
        T1,
        R1,
        B1,
        L2,
        T2,
        R2,
        B2,
        Size
    };

    enum DockMode
    {
        DockInNoneArea,
        DockInMainArea,
        DockInSideArea
    };

    enum ViewMode
    {
        ToolView,
        ToolPanesView,
        ToolPagesView,
        FileView,
        FilePanesView,
        FilePagesView,
        HybridView
    };

    enum Button
    {
        DockPull,
        AutoHide,
        DockShow,
        Minimize,
        Maximize,
        Restore,
        Close
    };

    enum Feature
    {
        AllowDockAsNorthTabPage = 0x1,
        AllowDockAsSouthTabPage = 0x2,
    };

    enum Direction
    {
        C = -1,
        L,
        T,
        R,
        B
    };

    Q_DECLARE_FLAGS(Features, Feature)

public:
    static inline Qt::WindowFlags widgetFlags()
    {
        return Qt::Widget;
    }

    static inline Qt::WindowFlags windowFlags()
    {
        return Qt::Window | Qt::CustomizeWindowHint;
    }

    static inline Qt::WindowFlags windowFlags(ViewMode viewMode)
    {
        switch (viewMode)
        {
        case ToolView:
            return Qt::Window | Qt::WindowMaximizeButtonHint;
        case ToolPanesView:
            return Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowSystemMenuHint;
        case ToolPagesView:
            return Qt::Window | Qt::WindowMaximizeButtonHint;
        case FileView:
            return Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint;
        case FilePanesView:
            return Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint;
        case FilePagesView:
            return Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint;
        case HybridView:
            return Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint;
        default:
            return Qt::Window;
        }
    }

    static QWidget* window();

    static inline QWidget* parent(ViewMode viewMode)
    {
        if (viewMode == Flex::ToolView || viewMode == Flex::ToolPanesView || viewMode == Flex::ToolPagesView)
        {
            return Flex::window();
        }
        else
        {
            return nullptr;
        }
    }

    static inline QWidget* parent(ViewMode viewMode, QWidget* widget)
    {
        if (viewMode == Flex::ToolView || viewMode == Flex::ToolPanesView || viewMode == Flex::ToolPagesView)
        {
            return widget->parentWidget() ? widget->parentWidget() : Flex::window();
        }
        else
        {
            return nullptr;
        }
    }

public:
    static int Update;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Flex::Features)

class FlexManagerImpl;

class QT_FLEX_API FlexManager : public QObject
{
    Q_OBJECT
protected:
    FlexManager();
    ~FlexManager();

Q_SIGNALS:
    void guiderHover(FlexWidget*, QWidget*);
    void guiderShow(FlexWidget*, QWidget*);
    void guiderHide(FlexWidget*, QWidget*);
    void guiderDrop(FlexWidget*, DockWidget*);
    void guiderDrop(FlexWidget*, FlexWidget*);

Q_SIGNALS:
    void dockSiteActivated(DockSite*);

public:
    FlexWidget* createFlexWidget(Flex::ViewMode viewMode, QWidget* parent, Qt::WindowFlags flags = Flex::widgetFlags());
    DockWidget* createDockWidget(Flex::ViewMode viewMode, QWidget* parent, Qt::WindowFlags flags = Flex::widgetFlags());

public:
    static FlexManager* instance();

public:
    QIcon icon(Flex::Button button);

private:
    bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:
    void on_app_focusChanged(QWidget* old, QWidget* now);
    void on_dockWidget_destroyed(QObject* widget);
    void on_flexWidget_destroyed(QObject* widget);
    void on_flexWidget_guiderShow(FlexWidget*, QWidget*);
    void on_flexWidget_guiderHide(FlexWidget*, QWidget*);
    void on_flexWidget_guiderHover(FlexWidget*, QWidget*);
    void on_flexWidget_guiderDrop(FlexWidget*, DockWidget*);
    void on_flexWidget_guiderDrop(FlexWidget*, FlexWidget*);
    void on_flexWidget_enterMove(QObject*);
    void on_flexWidget_leaveMove(QObject*);
    void on_flexWidget_moving(QObject*);
    void on_dockWidget_enterMove(QObject*);
    void on_dockWidget_leaveMove(QObject*);
    void on_dockWidget_moving(QObject*);

private:
    friend class FlexManagerImpl;

private:
    QScopedPointer<FlexManagerImpl> impl;
};

#endif

#include "QtGuider.h"
#include "QtFlexWidget.h"
#include "QtFlexHelper.h"
#include "QtDockGuider.h"
#include "QtDockWidget.h"
#include "QtFlexManager.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QSplitter>
#include "ui_qtguider.h"

class Central : public QWidget
{
public:
    Central(QWidget* parent) : QWidget(parent)
    {
        createOne();
    }
    void createOne()
    {
        auto content = FlexManager::instance()->createFlexWidget(Flex::HybridView, this, Flex::widgetFlags());
        QHBoxLayout* box = new QHBoxLayout(this);
        box->setContentsMargins(0, 0, 0, 0);
        box->addWidget(content);
    }
    void createTwo()
    {
        auto splitter = new QSplitter(this);
        auto l = FlexManager::instance()->createFlexWidget(Flex::HybridView, this, Flex::widgetFlags());
        auto r = FlexManager::instance()->createFlexWidget(Flex::HybridView, this, Flex::widgetFlags());
        splitter->addWidget(l);
        splitter->addWidget(r);
        QHBoxLayout* box = new QHBoxLayout(this);
        box->setContentsMargins(0, 0, 0, 0);
        box->addWidget(splitter);
    }
};

class QtGuiderImpl
{
public:
    Ui::QtGuiderClass ui;
};

QtGuider::QtGuider(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags), impl(new QtGuiderImpl)
{
    qApp->setProperty("window", QVariant::fromValue<QObject*>(this));

    impl->ui.setupUi(this);

    setCentralWidget(new Central(this));

    setGeometry(QRect(QApplication::desktop()->availableGeometry().center() - QPoint(512, 360), QSize(1024, 720)));
}

QtGuider::~QtGuider()
{

}

void QtGuider::on_action_Open_triggered()
{
    static int i = 0;
    FlexWidget* flexWidget = FlexManager::instance()->createFlexWidget(Flex::FileView, Flex::parent(Flex::FileView), Flex::windowFlags());
    DockWidget* dockWidget = FlexManager::instance()->createDockWidget(Flex::FileView, flexWidget);
    dockWidget->setViewMode(Flex::FileView);
    dockWidget->setWindowTitle(QString("Doc-%1").arg(i++));
    flexWidget->addDockWidget(dockWidget);
    flexWidget->show();
    flexWidget->move(geometry().center() - flexWidget->rect().center());
}

void QtGuider::on_action_Show_triggered()
{
    static int i = 0;
    FlexWidget* flexWidget = FlexManager::instance()->createFlexWidget(Flex::ToolView, Flex::parent(Flex::ToolView), Flex::windowFlags());
    DockWidget* dockWidget = FlexManager::instance()->createDockWidget(Flex::ToolView, flexWidget);
    dockWidget->setViewMode(Flex::ToolView);
    dockWidget->setWindowTitle(QString("View-%1").arg(i++));
    flexWidget->addDockWidget(dockWidget);
    flexWidget->show();
    flexWidget->move(geometry().center() - flexWidget->rect().center());
}

void QtGuider::on_action_Undo_triggered()
{
    static int i = 0;
    DockWidget* dockWidget = FlexManager::instance()->createDockWidget(Flex::FileView, Flex::parent(Flex::FileView), Flex::windowFlags());
    dockWidget->setViewMode(Flex::FileView);
    dockWidget->setWindowTitle(QString("Doc-%1").arg(i++));
    dockWidget->show();
    dockWidget->move(geometry().center() - dockWidget->rect().center());
}

void QtGuider::on_action_Redo_triggered()
{
    static int i = 0;
    DockWidget* dockWidget = FlexManager::instance()->createDockWidget(Flex::ToolView, Flex::parent(Flex::ToolView), Flex::windowFlags());
    dockWidget->setViewMode(Flex::ToolView);
    dockWidget->setWindowTitle(QString("Tool-%1").arg(i++));
    dockWidget->show();
    dockWidget->move(geometry().center() - dockWidget->rect().center());
}

void QtGuider::on_action_Exit_triggered()
{
    close();
}

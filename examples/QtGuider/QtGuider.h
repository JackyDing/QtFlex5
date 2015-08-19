#ifndef QTGUIDER_H
#define QTGUIDER_H

#include <QtWidgets/QMainWindow>

class QtGuiderImpl;

class QtGuider : public QMainWindow
{
    Q_OBJECT
public:
    QtGuider(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QtGuider();

private slots:
    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
    void on_action_Open_triggered();
    void on_action_Show_triggered();
    void on_action_Exit_triggered();

private:
    friend class QtGuiderImpl;
private:
    QScopedPointer<QtGuiderImpl> impl;
};

#endif // QTGUIDER_H

#ifndef QTDOCKSIDE_H
#define QTDOCKSIDE_H

#include "QtFlexManager.h"

class DockSideImpl;

class QT_FLEX_API DockSide : public QWidget
{
    Q_OBJECT
public:
    DockSide(Flex::Direction direction, QWidget* parent);
    ~DockSide();
    
Q_SIGNALS:
    void dockSiteAttached(DockSide*, DockSite*);
    void dockSiteDetached(DockSide*, DockSite*);
    void currentChanged(DockSide*, DockSite* prev, DockSite* curr);

public:
    Flex::Direction direction() const;

public:
    QSize sizeHint() const;

public:
    void setHeadOffset(int offset);
    void setTailOffset(int offset);
    void setSpace(int space);

public:
    bool attachDockSite(DockSite* dockSite);
    bool detachDockSite(DockSite* dockSite);

public:
    bool hasDockSite(DockSite* dockSite);

public:
    int count() const;
    DockSite* dockSite(int index) const;
    const QList<DockSite*>& dockSites() const;

public:
    DockSite* current() const;

public:
    void doneCurrent();

public:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

private:
    friend class DockSideImpl;
private:
    QScopedPointer<DockSideImpl> impl;
};

#endif

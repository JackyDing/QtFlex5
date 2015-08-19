#include "QtDockSide.h"
#include "QtDockSite.h"
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

class DockSideImpl
{
public:
    DockSideImpl() : _over(-1), _space(12), _headOffset(0), _tailOffset(0), _curr(-1)
    {
    }

public:
    void hittest(DockSide* self, const QPoint& pos);

public:
    int _over;
    int _space;
    int _headOffset;
    int _tailOffset;
    int _curr;
    Flex::Direction _direction;
    QList<DockSite*> _dockSites;
};

void DockSideImpl::hittest(DockSide* self, const QPoint& pos)
{
    int v = _direction == Flex::L || _direction == Flex::R ? pos.y() : pos.x();

    int h = _over;

    int offset = _headOffset;

    _over = -1;

    for (int i = 0; i < _dockSites.size(); ++i)
    {
        int width = self->fontMetrics().width(_dockSites[i]->windowTitle());

        if (v >= offset && v < offset + width)
        {
            _over = i; break;
        }

        offset += width + _space;
    }

    if (_over != h)
    {
        self->update();
    }
}

DockSide::DockSide(Flex::Direction direction, QWidget* parent) : QWidget(parent), impl(new DockSideImpl)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);

    impl->_direction = direction;

    switch (impl->_direction)
    {
    case Flex::L:
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        break;
    case Flex::T:
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        break;
    case Flex::R:
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        break;
    case Flex::B:
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        break;
    }
}

DockSide::~DockSide()
{

}

Flex::Direction DockSide::direction() const
{
    return impl->_direction;
}

bool DockSide::attachDockSite(DockSite* dockSite)
{
    doneCurrent();

    if (std::find(impl->_dockSites.begin(), impl->_dockSites.end(), dockSite) == impl->_dockSites.end())
    {
        impl->_dockSites.append(dockSite);
        dockSite->setDockMode(Flex::DockInSideArea);
        dockSite->hide();
        emit dockSiteAttached(this, dockSite);
        update();
    }

    impl->_over = -1;
    impl->_curr = -1;

    if (!impl->_dockSites.empty())
    {
        show();
    }

    return true;
}

bool DockSide::detachDockSite(DockSite* dockSite)
{
    doneCurrent();

    if (impl->_dockSites.removeOne(dockSite))
    {
        dockSite->setDockMode(Flex::DockInMainArea);
        dockSite->hide();
        emit dockSiteDetached(this, dockSite);
        update();
    }

    impl->_over = -1;
    impl->_curr = -1;

    if (impl->_dockSites.empty())
    {
        hide();
    }

    return true;
}

QSize DockSide::sizeHint() const
{
    return QSize(30, 30);
}

void DockSide::setHeadOffset(int offset)
{
    if (impl->_headOffset != offset)
    {
        impl->_headOffset = offset;
        update();
    }
}

void DockSide::setTailOffset(int offset)
{
    if (impl->_tailOffset != offset)
    {
        impl->_tailOffset = offset;
        update();
    }
}

void DockSide::setSpace(int space)
{
    if (impl->_space != space)
    {
        impl->_space = space;
        update();
    }
}

void DockSide::paintEvent(QPaintEvent*)
{
    auto dim = size();

    int pw = 0;
    int ph = 0;

    switch (impl->_direction)
    {
    case Flex::L:
    case Flex::R:
        pw = dim.height();
        ph = dim.width();
        break;
    case Flex::T:
    case Flex::B:
        pw = dim.width();
        ph = dim.height();
        break;
    }

    QPixmap pixmap(pw, ph);

    pixmap.fill(Qt::transparent);

    QPainter pixmapPainter(&pixmap);

    pixmapPainter.initFrom(this);

    int offset = impl->_headOffset;

    for (int i = 0; i < impl->_dockSites.size(); ++i)
    {
        auto dockText = impl->_dockSites[i]->windowTitle();
        auto dockRect = pixmapPainter.fontMetrics().boundingRect(dockText);
        switch (impl->_direction)
        {
        case Flex::L:
        case Flex::B:
            pixmapPainter.fillRect(offset, ph - 6, dockRect.width(), 6, impl->_over == i ? QColor("#9BA7B7") : QColor("#465A7D"));
            pixmapPainter.drawText(offset, ph - dockRect.height(), dockText);
            break;
        case Flex::T:
        case Flex::R:
            pixmapPainter.fillRect(offset, 0, dockRect.width(), 6, impl->_over == i ? QColor("#9BA7B7") : QColor("#465A7D"));
            pixmapPainter.drawText(offset, ph - dockRect.height(), dockText);
            break;
        }
        offset += dockRect.width() + impl->_space;
    }

    pixmapPainter.end();

    QPainter painter(this);

    if (impl->_direction == Flex::L || impl->_direction == Flex::R)
    {
        painter.translate(ph, 0);
        painter.rotate(90);
    }

    painter.drawPixmap(QRect(0, 0, pw, ph), pixmap);
}

void DockSide::mouseMoveEvent(QMouseEvent* evt)
{
    impl->hittest(this, evt->pos());
}

void DockSide::mousePressEvent(QMouseEvent* evt)
{
    impl->hittest(this, evt->pos());

    if (impl->_over != -1 && impl->_curr != impl->_over)
    {
        auto prev = impl->_curr != -1 ? impl->_dockSites[impl->_curr] : nullptr;
        auto curr = impl->_dockSites[impl->_over];

        impl->_curr = impl->_over;

        emit currentChanged(this, prev, curr);
    }
}

void DockSide::enterEvent(QEvent*)
{
    impl->hittest(this, mapFromGlobal(QCursor::pos()));
}

void DockSide::leaveEvent(QEvent*)
{
    impl->_over = -1; update();
}

bool DockSide::hasDockSite(DockSite* dockSite)
{
    return impl->_dockSites.contains(dockSite);
}

int DockSide::count() const
{
    return impl->_dockSites.size();
}

DockSite* DockSide::dockSite(int index) const
{
    return impl->_dockSites[index];
}

const QList<DockSite*>& DockSide::dockSites() const
{
    return impl->_dockSites;
}

DockSite* DockSide::current() const
{
    return impl->_curr != -1 ? impl->_dockSites[impl->_curr] : nullptr;
}

void DockSide::doneCurrent()
{
    if (impl->_curr != -1)
    {
        auto curr = impl->_dockSites[impl->_curr];
        impl->_curr = -1;
        emit currentChanged(this, curr, nullptr);
    }
}

#include "QtFlexStyle.h"
#include "QtFlexManager.h"
#include "QtFlexWidget.h"
#include "QtDockSite.h"
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOption>
#include <qdrawutil.h>

class FlexStyleImpl
{
public:
    FlexStyleImpl() : soImage(":/Resources/shadows.png")
    {
    }

public:
    QPixmap soImage;
};

FlexStyle::FlexStyle(QStyle* style) : QProxyStyle(style), impl(new FlexStyleImpl)
{
    setObjectName(QLatin1String("FlexStyle"));
}

FlexStyle::~FlexStyle()
{
}

void FlexStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const
{
    switch (pe)
    {
    case PE_IndicatorTabClose:
        if (!w || !w->parent()->property("Flex").isValid())
        {
            break;
        }
        if (w != nullptr)
        {
            bool active = w->parentWidget()->property("active").toBool();
            bool highlight = opt->state & State_Selected;
            bool mouseover = opt->state & State_MouseOver;
            bool mousedown = opt->state & QStyle::State_Sunken;

            QColor color;

            if (mousedown)
            {
                color = QColor("#0E6198");
            }
            else if (highlight)
            {
                color = QColor(active ? "#1C97EA" : "#555555");
            }
            else if (mouseover)
            {
                color = QColor(active ? "#52B0EF" : "#52B0EF");
            }
            else
            {
                color = QColor(active ? "#52B0EF" : "#555555");
            }

            p->setPen(color);
            p->setBrush(color);

            if (mouseover)
            {
                p->drawRect(opt->rect.adjusted(0, 0, -1, -1));
            }

            FlexManager::instance()->icon(Flex::Close).paint(p, opt->rect, Qt::AlignCenter, (mouseover || highlight) ? QIcon::Active : QIcon::Normal, mouseover ? QIcon::On : QIcon::Off);
        }
        return;
    case PE_Frame:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        if (auto tmp = qstyleoption_cast<const QStyleOptionFrame*>(opt))
        {
            if (w->isWindow())
            {
                p->setPen(QColor(w->property("active").toBool() ? "#007ACC" : "#3F3F46"));
            }
            else
            {
                p->setPen(QColor("#3F3F46"));
            }

            p->drawRect(opt->rect.adjusted(0, 0, -1, -1));

            return;
        }
        return;
    case PE_FrameTabBarBase:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        if (auto tbb = qstyleoption_cast<const QStyleOptionTabBarBase*>(opt))
        {
            auto active = w->property("active").toBool();
            auto colour = tbb->palette.color(active ? QPalette::Active : QPalette::Inactive, QPalette::Highlight);
            p->save();
            switch (tbb->shape)
            {
            case QTabBar::RoundedNorth:
                p->setPen(QPen(colour, 1));
                p->drawLine(tbb->rect.bottomLeft() + QPoint(0, -1), tbb->rect.bottomRight() + QPoint(0, -1));
                p->drawLine(tbb->rect.bottomLeft() + QPoint(0, +0), tbb->rect.bottomRight() + QPoint(0, +0));
                break;
            case QTabBar::RoundedSouth:
                p->setPen(QPen(QColor("#3F3F46"), 1));
                p->drawLine(tbb->rect.left(), tbb->rect.top() + 0, tbb->rect.right(), tbb->rect.top() + 0);
                break;
            default:
                break;
            }
            p->restore();
        }
        return;
    case PE_FrameWindow:
        if (!w || !w->property("Grow").isValid())
        {
            break;
        }
        if (opt != nullptr)
        {
            QRect rect = w->rect();

            int offset = opt->state & State_Active ? 16 : 0;

            p->fillRect(rect, Qt::transparent);

            switch (w->property("Grow").toInt())
            {
            case 0:
                qDrawBorderPixmap(p, rect, QMargins(0, 5, 0, 5), impl->soImage, QRect(0, offset, 8, 16), QMargins(0, 5, 0, 5));
                break;
            case 1:
                qDrawBorderPixmap(p, rect, QMargins(13, 0, 13, 0), impl->soImage, QRect(8, offset, 31, 8), QMargins(13, 0, 13, 0));
                break;
            case 2:
                qDrawBorderPixmap(p, rect, QMargins(0, 5, 0, 5), impl->soImage, QRect(39, offset, 8, 16), QMargins(0, 5, 0, 5));
                break;
            case 3:
                qDrawBorderPixmap(p, rect, QMargins(13, 0, 13, 0), impl->soImage, QRect(8, offset + 8, 31, 8), QMargins(13, 0, 13, 0));
                break;
            }
            return;
        }
        return;
    default:
        break;
    }
    QProxyStyle::drawPrimitive(pe, opt, p, w);
}

void FlexStyle::drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const
{
    switch (ce)
    {
    case CE_TabBarTabShape:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        if (auto tab = qstyleoption_cast<const QStyleOptionTab*>(opt))
        {
            bool highlight = tab->state & State_Selected;
            bool mouseover = tab->state & State_MouseOver;
            bool active = w->property("active").toBool();

            QRect rect = tab->rect;

            switch (tab->shape)
            {
            case QTabBar::RoundedNorth:
                rect.adjust(0, 0, 0, highlight ? -2 : -3);
                break;
            case QTabBar::RoundedSouth:
                rect.adjust(0, highlight ? -1 : 1, 0, highlight ? -1 : 1);
                break;
            default:
                break;
            }

            QColor bgColor;

            if (highlight)
            {
                bgColor = tab->palette.color(active ? QPalette::Active : QPalette::Inactive, QPalette::Highlight);
            }
            else if (mouseover)
            {
                bgColor = tab->palette.color(QPalette::Active, w->backgroundRole());
            }
            else
            {
                bgColor = tab->palette.color(QPalette::Inactive, w->backgroundRole());
            }

            p->setBrush(bgColor);

            p->setPen(tab->shape == QTabBar::RoundedSouth && highlight ? QColor("#3F3F46") : bgColor);

            p->drawRect(rect);

            //if (tab->shape == QTabBar::RoundedSouth && !highlight)
            //{
            //    p->setPen(QColor("#4B5C74"));
            //    p->drawLine(tab->rect.bottomLeft() - QPoint(0, 0), tab->rect.bottomRight() - QPoint(0, 0));
            //    p->drawLine(tab->rect.bottomLeft() - QPoint(0, 1), tab->rect.bottomRight() - QPoint(0, 1));
            //}

            return;
        }
        break;
    case CE_TabBarTabLabel:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        if (auto tab = qstyleoption_cast<const QStyleOptionTab*>(opt))
        {
            bool active = w->property("active").toBool();
            bool highlight = tab->state & State_Selected;
            auto palette = tab->palette;
            if (tab->shape == QTabBar::RoundedNorth)
            {
                palette.setCurrentColorGroup(active && highlight ? QPalette::Active : QPalette::Inactive);
            }
            else
            {
                palette.setCurrentColorGroup(highlight ? QPalette::Active : QPalette::Inactive);
            }
            QStyleOptionTab tmp = *tab;
            tmp.palette = palette;
#ifdef Q_OS_MAC
            QCommonStyle::drawControl(ce, &tmp, p, w);
#else
            QProxyStyle::drawControl(ce, &tmp, p, w);
#endif
            return;
        }
        break;
    case CE_Splitter:
        if (!w || !w->parent()->property("Flex").isValid())
        {
            break;
        }
        if (w != nullptr)
        {
            return;
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawControl(ce, opt, p, w);
}

int FlexStyle::pixelMetric(PixelMetric pm, const QStyleOption* opt, const QWidget* w) const
{
    switch (pm)
    {
    case PM_TitleBarHeight:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        return 27;
    case PM_TabBarTabShiftVertical:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        return 0;
    case PM_TabBarTabHSpace:
        if (!w || !w->property("Flex").isValid())
        {
            break;
        }
        return 8;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(pm, opt, w);
}
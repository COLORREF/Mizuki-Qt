//
// Created by TX on 2026/6/25.
// 移植自 QWidget-FancyUI /src/Fancy/Widgets/ScrollBar/
//

#include "ScrollBarStyle.h"

#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyleOptionComplex>

#include "Core/Defs.h"
#include "Core/Theme/ThemeModeController.h"

// ─────────────────────────────────────────────
// 硬编码颜色（对齐 FancyUI ColorProfile.json）
// 始终绘制"激活"态，无关鼠标操作，故不再区分 Normal/Hover
// ─────────────────────────────────────────────
namespace ScrollBarColor
{
    inline constexpr auto Track = QColor(0xF9, 0xF9, 0xF9); // Light
    inline constexpr auto TrackDark = QColor(0x2C, 0x2C, 0x2C);
    inline constexpr auto ThumbLight = QColor(0x8A, 0x8A, 0x8A);
    inline constexpr auto ThumbDark = QColor(0x9F, 0x9F, 0x9F);
}

static QColor thumbColor()
{
    using namespace ScrollBarColor;
    return ThemeModeController::controller().isAppDark() ? ThumbDark : ThumbLight;
}

// ══════════════════════════════════════════════
//  ScrollBarStyle
// ══════════════════════════════════════════════

ScrollBarStyle::ScrollBarStyle(QScrollBar *parent) :
    QProxyStyle(nullptr)
{
    setParent(parent);
    parent->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void ScrollBarStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (control != CC_ScrollBar)
    {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    const auto *opt = qstyleoption_cast<const QStyleOptionSlider *>(option);
    if (!opt)
    {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    const bool isHorizontal = (opt->orientation == Qt::Horizontal);
    constexpr int sliderThick = 9;
    constexpr int sliderCenterOff = 4;

    const QRectF sliderRect = subControlRect(control, option, SC_ScrollBarSlider, widget);
    const QRectF grooveRect = subControlRect(control, option, SC_ScrollBarGroove, widget);
    const QRectF rect = opt->rect;

    const qreal center = isHorizontal ? sliderRect.center().y() : sliderRect.center().x();

    const QRectF addArrowRect = isHorizontal
                                    ? QRectF{grooveRect.topRight(), rect.bottomRight()}.adjusted(8, 4, -3, -4)
                                    : QRectF{grooveRect.bottomLeft(), rect.bottomRight()}.adjusted(4, 6, -4, -5);
    const QRectF subArrowRect = isHorizontal
                                    ? QRectF{rect.topLeft(), grooveRect.bottomLeft()}.adjusted(3, 4, -8, -4)
                                    : QRectF{rect.topLeft(), grooveRect.topRight()}.adjusted(4, 5, -4, -6);

    // 滑块（两端各缩 1px，增大与箭头间距）
    const QRectF slider = isHorizontal
                              ? QRectF(sliderRect.left() + 1, center - sliderCenterOff, sliderRect.width() - 2, sliderThick)
                              : QRectF(center - sliderCenterOff, sliderRect.top() + 1, sliderThick, sliderRect.height() - 2);

    const qreal sliderRadius = (isHorizontal ? slider.height() : slider.width()) / 2.0;

    const auto drawArrows = [painter, &addArrowRect, &subArrowRect](const bool horizontal) {
        if (horizontal)
        {
            drawArrow(painter, addArrowRect, Direction::Right);
            drawArrow(painter, subArrowRect, Direction::Left);
        }
        else
        {
            drawArrow(painter, addArrowRect, Direction::Down);
            drawArrow(painter, subArrowRect, Direction::Up);
        }
    };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    // ── 轨道背景（直角） ──
    {
        using namespace ScrollBarColor;
        painter->setBrush(
            ThemeModeController::controller().isAppDark() ? TrackDark : Track
        );
    }
    painter->drawRect(rect);

    // ── 滑块 ──
    painter->setBrush(thumbColor());
    painter->drawRoundedRect(slider, sliderRadius, sliderRadius);

    // ── 末端箭头 ──
    drawArrows(isHorizontal);

    painter->restore();
}

int ScrollBarStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
        case PM_ScrollBarExtent :
            return 15;
        case PM_ScrollBarSliderMin :
            return 18;
        default :
            break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void ScrollBarStyle::drawArrow(QPainter *painter, const QRectF &arrowRect, Direction dir)
{
    QPainterPath path;
    QPointF p1, p2, p3;

    switch (dir)
    {
        case Direction::Right :
            p1 = arrowRect.topLeft();
            p2 = arrowRect.bottomLeft();
            p3 = QPointF(arrowRect.right(), arrowRect.center().y());
            break;
        case Direction::Left :
            p1 = arrowRect.topRight();
            p2 = arrowRect.bottomRight();
            p3 = QPointF(arrowRect.left(), arrowRect.center().y());
            break;
        case Direction::Down :
            p1 = arrowRect.topLeft();
            p2 = arrowRect.topRight();
            p3 = QPointF(arrowRect.center().x(), arrowRect.bottom());
            break;
        case Direction::Up :
            p1 = arrowRect.bottomLeft();
            p2 = arrowRect.bottomRight();
            p3 = QPointF(arrowRect.center().x(), arrowRect.top());
            break;
        default :
            return;
    }

    path.moveTo(p1);
    path.lineTo(p2);
    path.lineTo(p3);
    path.closeSubpath();

    const QColor arrowColor = thumbColor();
    painter->fillPath(path, arrowColor);
    painter->strokePath(path, QPen(arrowColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

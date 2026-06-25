//
// Created by TX on 2026/6/25.
// 移植自 QWidget-FancyUI /src/Fancy/Widgets/ScrollBar/
//

#ifndef MIZUKI_QT_SCROLLBARSTYLE_H
#define MIZUKI_QT_SCROLLBARSTYLE_H

#include <QProxyStyle>

enum class Direction;
class QScrollBar;

class ScrollBarStyle final : public QProxyStyle
{
    Q_OBJECT

public:
    explicit ScrollBarStyle(QScrollBar *parent);

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override;

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;

    static void drawArrow(QPainter *painter, const QRectF &arrowRect, Direction dir);
};

#endif //MIZUKI_QT_SCROLLBARSTYLE_H

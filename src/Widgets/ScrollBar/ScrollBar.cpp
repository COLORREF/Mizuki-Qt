//
// Created by TX on 2026/6/25.
// 移植自 QWidget-FancyUI /src/Fancy/Widgets/ScrollBar/
//

#include "ScrollBar.h"
#include "ScrollBarStyle.h"

ScrollBar::ScrollBar(QWidget *parent) :
    QScrollBar(parent)
{
    setStyle(new ScrollBarStyle(this));
}

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget *parent) :
    QScrollBar(orientation, parent)
{
    setStyle(new ScrollBarStyle(this));
}

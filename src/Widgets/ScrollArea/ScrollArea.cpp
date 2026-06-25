//
// Created by TX on 2026/6/12.
//

#include "ScrollArea.h"

#include "ScrollAreaStyle.h"
#include "Widgets//ScrollBar/ScrollBar.h"

ScrollArea::ScrollArea(QWidget *parent) :
    QScrollArea(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Window, Qt::transparent);
    setPalette(p);
    setStyle(new ScrollAreaStyle(this));

    // 替换默认滚动条为自定义圆角滚动条
    setVerticalScrollBar(new ScrollBar(Qt::Vertical, this));
    setHorizontalScrollBar(new ScrollBar(Qt::Horizontal, this));
}

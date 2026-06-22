//
// Created by TX on 2026/6/12.
//

#include "ScrollArea.h"

#include "ScrollAreaStyle.h"

ScrollArea::ScrollArea(QWidget *parent) :
    QScrollArea(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Window, Qt::transparent);
    setPalette(p);
    setStyle(new ScrollAreaStyle(this));

    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

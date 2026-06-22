//
// Created by TX on 2026/6/13.
//

#include "ScrollAreaStyle.h"

#include <QScrollArea>


ScrollAreaStyle::ScrollAreaStyle(QScrollArea *parent) :
    QProxyStyle(nullptr)
{
    setParent(parent);
}

void ScrollAreaStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return;
}

void ScrollAreaStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return;
}

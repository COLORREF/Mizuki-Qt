//
// Created by TX on 2026/6/26.
//

#include "MizukiButtonStyle.h"

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyleOption>

#include "Core/Defs.h"
#include "Core/Paltette/Palette.h"

MizukiButtonStyle::MizukiButtonStyle(QPushButton *parent) :
    QProxyStyle(nullptr),
    _buttonRadius(8)
{
    setParent(parent);
    connect(&Palette::instance(), &Palette::appColorChange, parent, QOverload<>::of(&QPushButton::update));
}

void MizukiButtonStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_PushButtonBevel)
    {
        // ControlState 直接在代码中展开，不引入额外类型
        const bool isDown = option->state & QStyle::State_Sunken;
        const bool isHover = option->state & QStyle::State_MouseOver;

        ColorRole bgRole;
        if (isDown)
            bgRole = ColorRole::BtnRegularBgActive;
        else if (isHover)
            bgRole = ColorRole::BtnRegularBgHover;
        else
            bgRole = ColorRole::BtnRegularBg;

        painter->save();
        painter->setRenderHint(QPainter::RenderHint::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(option->rect, _buttonRadius, _buttonRadius);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Palette::instance()[bgRole]);
        painter->drawPath(path);

        painter->restore();
        return;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void MizukiButtonStyle::setCornerRadius(const int radius)
{
    _buttonRadius = radius;
}

int MizukiButtonStyle::cornerRadius() const
{
    return _buttonRadius;
}

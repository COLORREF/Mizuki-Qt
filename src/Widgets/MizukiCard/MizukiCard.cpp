//
// Created by TX on 2026/6/26.
//

#include "MizukiCard.h"

#include <QPainter>
#include <QPainterPath>

#include "Core/Paltette/Palette.h"
#include "Core/Theme/ThemeModeController.h"

MizukiCard::MizukiCard(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAutoFillBackground(false);

    // 主题 / 色相切换 → 重绘
    connect(&Palette::instance(), &Palette::appColorChange, this, QOverload<>::of(&MizukiCard::update));
    connect(&ThemeModeController::controller(), &ThemeModeController::appThemeChange, this, QOverload<>::of(&MizukiCard::update));
}

void MizukiCard::setCornerRadius(const int radius)
{
    _cornerRadius = radius;
    update();
}

int MizukiCard::cornerRadius() const
{
    return _cornerRadius;
}

// ── 绘制圆角背景 + 边框（颜色从 Palette 获取） ──
void MizukiCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    const QRectF r = rect().toRectF().adjusted(0, 0, -0.5, -0.5);

    // 背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(r, _cornerRadius, _cornerRadius);
    painter.fillPath(bgPath, Palette::instance()[ColorRole::CardBg]);

    // 边框
    const QPen borderPen(Palette::instance()[ColorRole::LineDivider], 1.0);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(r, _cornerRadius, _cornerRadius);
}

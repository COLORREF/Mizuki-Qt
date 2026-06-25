//
// Created by TX on 2026/6/23.
//

#include "AvatarButton.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>

#include "Core/SizeManager/SizeManager.h"

constexpr qreal kCornerRadius = 12.0; // 圆角半径，对齐 Mizuki rounded-xl

AvatarButton::AvatarButton(QWidget *parent) :
    QPushButton(parent)
{
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &AvatarButton::recalcSizeAndPixmap);
}

void AvatarButton::setAvatarPixmap(const QPixmap &pixmap)
{
    _sourcePixmap = pixmap;
}

void AvatarButton::recalcSizeAndPixmap()
{
    const auto window_w = static_cast<qreal>(window()->width());

    int margin; // 父布局边距
    if (window_w <= 1280.0 || window_w > 1946.0)
        margin = 12;
    else if (window_w < 1765.0)
        margin = 10;
    else
        margin = 11;

    qreal bar_width; // 侧边栏宽度 = ProFile宽度
    if (window_w <= 1280.0 || window_w >= 2015.0)
        bar_width = 280.0;
    else if (window_w <= 1715.0)
        bar_width = 238.0;
    else
        bar_width = 0.139766 * window_w - 1.65575;
    const int bar_width_i = qRound(bar_width);

    const int wh = bar_width_i - margin - margin;
    setFixedHeight(wh);

    if (!_sourcePixmap.isNull())
        _scaledPixmap = _sourcePixmap.scaled(
            QSize(wh, wh),
            Qt::AspectRatioMode::KeepAspectRatioByExpanding,
            Qt::TransformationMode::SmoothTransformation
        );

    update();
}

void AvatarButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 圆角裁剪路径
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), kCornerRadius, kCornerRadius);
    painter.setClipPath(clipPath);

    // 白色背景
    painter.fillRect(rect(), Qt::GlobalColor::white);

    if (_scaledPixmap.isNull())
        return;

    // 等比放大填充后居中绘制，超出部分由 clipPath 裁剪为圆角
    const int x = (width() - _scaledPixmap.width()) / 2;
    const int y = (height() - _scaledPixmap.height()) / 2;
    painter.drawPixmap(x, y, _scaledPixmap);
}

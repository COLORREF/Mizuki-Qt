//
// Created by TX on 2026/6/23.
//

#include "ProfileCard.h"
#include "AvatarButton.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

#include "Core/Paltette/Palette.h"
#include "Core/ResourceManager/ResourceManager.h"
#include "Core/SizeManager/SizeManager.h"
#include "Core/Theme/ThemeModeController.h"

constexpr int kCornerRadius = 16;

ProfileCard::ProfileCard(QWidget *parent) :
    QWidget(parent),
    _cardLayout(new QVBoxLayout(this)),
    _avatar(new AvatarButton(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAutoFillBackground(false);

    _cardLayout->setContentsMargins(10, 10, 10, 10);
    _cardLayout->setSpacing(10);
    _cardLayout->addWidget(_avatar);

    // 异步加载头像（后端请求 → 失败回退本地 qrc 资源）
    connect(&ResourceManager::manager(), &ResourceManager::avatarReady, _avatar, &AvatarButton::setAvatarPixmap);
    ResourceManager::manager().loadAvatar();

    // 色相 / 主题切换 → 重绘
    connect(&Palette::instance(), &Palette::appColorChange, this, QOverload<>::of(&ProfileCard::update));
    connect(&ThemeModeController::controller(), &ThemeModeController::appThemeChange, this, QOverload<>::of(&ProfileCard::update));
    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &ProfileCard::reSetLayout);
}

// ── 绘制圆角卡片背景 + 边框（颜色直接从 Palette 获取） ──
void ProfileCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    const QRectF r = rect().toRectF().adjusted(0, 0, -0.5, -0.5);

    // 背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(r, kCornerRadius, kCornerRadius);
    painter.fillPath(bgPath, Palette::instance()[ColorRole::CardBg]);

    // 边框
    const QPen borderPen(Palette::instance()[ColorRole::LineDivider], 1.0);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(r, kCornerRadius, kCornerRadius);
}

void ProfileCard::reSetLayout()
{
    const auto window_w = static_cast<qreal>(window()->width());

    int margin;
    if (window_w <= 1280.0 || window_w > 1946.0)
        margin = 12;
    else if (window_w < 1765.0)
        margin = 10;
    else
        margin = 11;

    _cardLayout->setContentsMargins(margin, margin, margin, margin);
    _cardLayout->setSpacing(margin);

    update();
}

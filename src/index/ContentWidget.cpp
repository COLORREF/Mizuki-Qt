//
// Created by TX on 2026/6/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ContentWidget.h" resolved

#include "ContentWidget.h"

#include <algorithm>

#include <QPainter>
#include <QVBoxLayout>

#include "Core/Paltette/Palette.h"
#include "Core/SizeManager/SizeManager.h"
#include "Core/Theme/ThemeModeController.h"
#include "Widgets/ProfileCard/ProfileCard.h"

ContentWidget::ContentWidget(QWidget *parent) :
    QWidget(parent),
    _contentLayout(new QHBoxLayout(this)),
    _parcel(new QWidget(this)),
    _parcelLayout(new QHBoxLayout(_parcel)),
    _leftBar(new QWidget(_parcel)),
    _middleBar(new QWidget(_parcel)),
    _rightBar(new QWidget(_parcel)),
    _leftBarLayout(new QVBoxLayout(_leftBar)),
    _profileCard(new ProfileCard(_leftBar))
{
    _contentLayout->setContentsMargins(0, 0, 0, 0);
    _contentLayout->setSpacing(0);

    constexpr QSizePolicy parcel_policy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    _parcel->setSizePolicy(parcel_policy);
    _parcel->setMaximumWidth(1441);

    _parcelLayout->setSpacing(15);
    _parcelLayout->setContentsMargins(13, 0, 13, 0);

    constexpr QSizePolicy leftBar_policy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
    _leftBar->setSizePolicy(leftBar_policy);
    constexpr QSizePolicy middleBar_policy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    _middleBar->setSizePolicy(middleBar_policy);
    constexpr QSizePolicy rightBar_policy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
    _rightBar->setSizePolicy(rightBar_policy);

    _parcelLayout->addWidget(_leftBar);
    _parcelLayout->addWidget(_middleBar);
    _parcelLayout->addWidget(_rightBar);

    auto *spacer_left = new QSpacerItem(40, 20, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    auto *spacer_right = new QSpacerItem(40, 20, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    _contentLayout->addItem(spacer_left);
    _contentLayout->addWidget(_parcel);
    _contentLayout->addItem(spacer_right);


    _leftBarLayout->setContentsMargins(0, 0, 0, 0);
    _leftBarLayout->setSpacing(0);
    _leftBarLayout->addWidget(_profileCard);
    _leftBarLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding));

    // 色相 / 主题切换 → 重绘背景
    connect(&Palette::instance(), &Palette::appColorChange, this, QOverload<>::of(&ContentWidget::update));
    connect(&ThemeModeController::controller(), &ThemeModeController::appThemeChange, this, QOverload<>::of(&ContentWidget::update));
    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &ContentWidget::setSize);
}

ContentWidget::~ContentWidget() {}

void ContentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::PenStyle::NoPen);
    painter.fillRect(rect(), Palette::instance()[ColorRole::PageBg]);
}

void ContentWidget::setSize() const
{
    const auto window_w = static_cast<qreal>(window()->width());
    const int spacing = std::clamp(qRound(window_w * (17.f / 1441.f)), 15, 17);

    int margin;
    if (window_w <= 1280.0 || window_w >= 1945.0)
        margin = 16;
    else if (window_w >= 1290.0 && window_w < 1715.0)
        margin = 13;
    else if (window_w >= 1835.0)
        margin = 15;
    else
        margin = 14;

    _parcelLayout->setSpacing(spacing);
    _parcelLayout->setContentsMargins(margin, 0, margin, 0);

    qreal bar_width; // 侧边栏宽度
    if (window_w <= 1280.0 || window_w >= 2015.0)
        bar_width = 280.0;
    else if (window_w <= 1715.0)
        bar_width = 238.0;
    else
        bar_width = 0.139766 * window_w - 1.65575;

    const int bar_width_i = qRound(bar_width);
    _leftBar->setFixedWidth(bar_width_i);
    _rightBar->setFixedWidth(bar_width_i);
}

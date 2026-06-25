//
// Created by TX on 2026/6/23.
//

#include "ProfileCard.h"
#include "AvatarButton.h"

#include <QPainter>
#include <QVBoxLayout>
#include "Core/SizeManager/SizeManager.h"

ProfileCard::ProfileCard(QWidget *parent) :
    QWidget(parent),
    _cardLayout(new QVBoxLayout(this)),
    _avatar(new AvatarButton(this))

{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _cardLayout->setContentsMargins(10, 10, 10, 10);
    _cardLayout->setSpacing(10);
    _cardLayout->addWidget(_avatar);

    _avatar->setAvatarPixmap(QPixmap(":/image/avatar.jpg"));

    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &ProfileCard::reSetLayout);
}

void ProfileCard::paintEvent(QPaintEvent *event)
{
    // QWidget::paintEvent(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::red);
}


void ProfileCard::reSetLayout() const
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
}

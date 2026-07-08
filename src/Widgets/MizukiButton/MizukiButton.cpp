//
// Created by TX on 2026/6/26.
//

#include "MizukiButton.h"
#include "MizukiButtonStyle.h"

#include <QtMath>

MizukiButton::MizukiButton(QWidget *parent) :
    QPushButton(parent)
{
    setStyle(new MizukiButtonStyle(this));
    setCursor(Qt::PointingHandCursor);
}

void MizukiButton::setCornerRadius(const int radius)
{
    if (auto *s = qobject_cast<MizukiButtonStyle *>(style()))
        s->setCornerRadius(radius);
}

int MizukiButton::cornerRadius() const
{
    if (auto *s = qobject_cast<MizukiButtonStyle *>(style()))
        return s->cornerRadius();
    return 0;
}
//
// Created by TX on 2026/6/25.
// 移植自 QWidget-FancyUI /src/Fancy/Widgets/ScrollBar/
//

#ifndef MIZUKI_QT_SCROLLBAR_H
#define MIZUKI_QT_SCROLLBAR_H

#include <QScrollBar>

class ScrollBar final : public QScrollBar
{
    Q_OBJECT

public:
    explicit ScrollBar(QWidget *parent = nullptr);
    explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);
};

#endif //MIZUKI_QT_SCROLLBAR_H

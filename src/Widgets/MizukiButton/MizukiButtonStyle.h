//
// Created by TX on 2026/6/26.
//

#ifndef MIZUKI_QT_MIZUKIBUTTONSTYLE_H
#define MIZUKI_QT_MIZUKIBUTTONSTYLE_H

#include <QProxyStyle>

class QPushButton;

/**
 * MizukiButton 的绘制代理 — 负责圆角背景绘制
 *
 * 参照 QWidget-FancyUI PushButtonStyle 模式，将绘制逻辑与按钮类分离。
 * 重写 drawControl(CE_PushButtonBevel) 拦截按钮斜面绘制，改为圆角背景。
 * ControlState 状态判断直接在代码中展开（isDown / isHover），不引入额外类型。
 */
class MizukiButtonStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit MizukiButtonStyle(QPushButton *parent);

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

    void setCornerRadius(int radius);

    [[nodiscard]] int cornerRadius() const;

private:
    int _buttonRadius = 8;
};

#endif // MIZUKI_QT_MIZUKIBUTTONSTYLE_H

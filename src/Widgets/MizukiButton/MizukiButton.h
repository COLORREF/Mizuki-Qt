//
// Created by TX on 2026/6/26.
//

#ifndef MIZUKI_QT_MIZUKIBUTTON_H
#define MIZUKI_QT_MIZUKIBUTTON_H

#include <QPushButton>

/**
 * 通用圆角按钮 — 绘制逻辑委托给 MizukiButtonStyle（QProxyStyle）
 *
 * 参照 QWidget-FancyUI PushButton 模式，按钮本身只负责构造和属性委托。
 * 圆角背景绘制、三态颜色（normal / hover / active）均由 MizukiButtonStyle 处理。
 *
 * 参考 Mizuki 项目 btn-regular：
 *   - 圆角 rounded-lg = 0.5rem (默认 8px)
 *   - bg-(--btn-regular-bg) / hover / active
 *   - transition-colors duration-150
 */
class MizukiButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MizukiButton(QWidget *parent = nullptr);

    /// 圆角半径（默认 8px，对齐 rounded-lg）
    void setCornerRadius(int radius);

    [[nodiscard]] int cornerRadius() const;
};

#endif // MIZUKI_QT_MIZUKIBUTTON_H

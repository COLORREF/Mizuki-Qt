//
// Created by TX on 2026/6/26.
//

#ifndef MIZUKI_QT_MIZUKICARD_H
#define MIZUKI_QT_MIZUKICARD_H

#include <QWidget>

/**
 * 通用圆角卡片基类 — 仅负责绘制背景 + 边框
 *
 * 职责：
 *   - paintEvent 绘制圆角背景（CardBg）与边框（LineDivider）
 *   - 颜色自动跟随 Palette / 主题切换
 *   - 不包含任何布局或子控件逻辑
 *
 * 子类只需关注内容布局，无需关心背景绘制。
 */
class MizukiCard : public QWidget
{
    Q_OBJECT

public:
    explicit MizukiCard(QWidget *parent = nullptr);

    /// 圆角半径（默认 16px）
    void setCornerRadius(int radius);
    int cornerRadius() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int _cornerRadius = 16;
};

#endif // MIZUKI_QT_MIZUKICARD_H

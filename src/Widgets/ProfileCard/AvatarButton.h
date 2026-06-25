//
// Created by TX on 2026/6/23.
//

#ifndef MIZUKI_QT_AVATARBUTTON_H
#define MIZUKI_QT_AVATARBUTTON_H

#include <QPushButton>

/**
 * 头像按钮 — 圆形图片 + 可点击跳转
 *
 * 参考 Mizuki 博客 Profile 卡片的头像区域：
 *   - 不绘制 QPushButton 默认样式，只在 paintEvent 中绘制图片
 *   - 12px 圆角、等比缩放居中显示
 *   - 缩放操作由外部（ProfileCard）驱动，仅在窗口大小调整结束后执行
 */
class AvatarButton : public QPushButton
{
    Q_OBJECT

public:
    explicit AvatarButton(QWidget *parent);

    /// 设置原始头像图片
    void setAvatarPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    /// SizeManager 驱动：先设固定尺寸，再缩放图片缓存，最后刷新
    void recalcSizeAndPixmap();

private:
    QPixmap _sourcePixmap;   // 原始图片
    QPixmap _scaledPixmap;   // 缩放缓存
};

#endif //MIZUKI_QT_AVATARBUTTON_H

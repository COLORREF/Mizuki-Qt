//
// Created by TX on 2025/10/15.
//

#ifndef GITHUBPAGE_BANNERCAROUSELWIDGET_H
#define GITHUBPAGE_BANNERCAROUSELWIDGET_H
#include <QPainterPath>
#include <QTimer>
#include <QWidget>

class QVariantAnimation;

class BannerCarouselWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannerCarouselWidget(QWidget *parent = nullptr);

public slots:
    void setPixmap(const QPixmap &pixmap);

signals:
    /// 新图已开始播放，需要外部预加载下一张到备用槽位
    void nextImageNeeded();

protected:
    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap _pixmaps[2]; // 两张图的原图（由外部 setPixmap 填入，自动选槽位）
    QPixmap _scaledMax[2]; // 预缩放缓存
    qreal _currentScale[2] = {1.0, 1.0}; // 缩放系数
    qreal _opacity = 1.0; // 透明度
    QTimer _debounceTimer; // 防抖计时器
    QPainterPath _painterPath;
    QVariantAnimation *_zoomAni[2]; // 放大动画
    QVariantAnimation *_opacityAni; // 透明度动画
    QTimer _fadeInOutTimer; // 淡出触发计时器
    bool _current = false; // 当前图片槽位
    int _fadeOutIdx = -1; // 正在淡出的图片槽位（-1 表示无淡出进行中）

private slots:
    void onZoom0Changed(const QVariant &value);

    void onZoom1Changed(const QVariant &value);

    void onOpacityChanged(const QVariant &value);

    void onOpacityFinished();

    void restartCurrentAnimation(); // 仅重置当前图预缩放 + 动画（不触发预加载）

    void startZoomAnimation(); // 完整轮播启动 = 重置动画 + emit nextImageNeeded

    void onFadeOutStart();
};

#endif

//
// Created by TX on 2025/10/15.
//

#include "BannerCarouselWidget.h"

#include <QPainter>
#include <QVariantAnimation>

constexpr int aniDuration = 4200;
constexpr qreal kZoomStart = 1.03;
constexpr qreal kZoomEnd = 1.13;
constexpr int fadeOutDuration = 1200;
constexpr int kOpacityFadeOutStart = 3000;


BannerCarouselWidget::BannerCarouselWidget(QWidget *parent) :
    QWidget(parent),
    _zoomAni{new QVariantAnimation(this), new QVariantAnimation(this)},
    _opacityAni{new QVariantAnimation(this)}
{
    // 防抖定时器 — resize 时仅重置动画，不触发预加载
    _debounceTimer.setSingleShot(true);
    _debounceTimer.setInterval(100);
    connect(&_debounceTimer, &QTimer::timeout, this, &BannerCarouselWidget::restartCurrentAnimation);

    // 两张图各自的放大动画
    for (const auto i: _zoomAni)
    {
        i->setDuration(aniDuration);
        i->setStartValue(kZoomStart);
        i->setEndValue(kZoomEnd);
    }
    connect(_zoomAni[0], &QVariantAnimation::valueChanged, this, &BannerCarouselWidget::onZoom0Changed);
    connect(_zoomAni[1], &QVariantAnimation::valueChanged, this, &BannerCarouselWidget::onZoom1Changed);

    // 透明度动画
    _opacityAni->setDuration(fadeOutDuration);
    _opacityAni->setStartValue(1.0);
    _opacityAni->setEndValue(0.0);
    connect(_opacityAni, &QVariantAnimation::valueChanged, this, &BannerCarouselWidget::onOpacityChanged);
    connect(_opacityAni, &QVariantAnimation::finished, this, &BannerCarouselWidget::onOpacityFinished);

    // 淡入淡出计时器：3000ms 后触发图片切换
    _fadeInOutTimer.setSingleShot(true);
    _fadeInOutTimer.setInterval(kOpacityFadeOutStart);
    connect(&_fadeInOutTimer, &QTimer::timeout, this, &BannerCarouselWidget::onFadeOutStart);
}

void BannerCarouselWidget::setPixmap(const QPixmap &pixmap)
{
    // 首次填入当前槽位，后续填入备用槽位（覆盖旧图）
    const int slot = _pixmaps[_current].isNull() ? _current : (1 - _current);
    _pixmaps[slot] = pixmap;

    // 首张图到达且动画尚未启动 → 正式开播
    if (slot == static_cast<int>(_current) && _zoomAni[_current]->state() != QAbstractAnimation::Running)
        startZoomAnimation();
}

void BannerCarouselWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    _fadeInOutTimer.stop();
    _opacityAni->stop();
    for (const auto i: _zoomAni)
        i->stop();
    _fadeOutIdx = -1;
    _debounceTimer.start();
}

void BannerCarouselWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    // 淡出期间两张图重叠，关闭抗锯齿和平滑变换以提升性能（视觉几乎无差异）
    if (_fadeOutIdx < 0)
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setClipPath(_painterPath);

    // 先画新图（底层），再画正在淡出的旧图（上层）
    for (const int idx: {static_cast<int>(_current), _fadeOutIdx})
    {
        if (idx < 0 || _scaledMax[idx].isNull())
            continue;

        const qreal drawOpacity = (idx == _fadeOutIdx) ? _opacity : 1.0;
        if (drawOpacity <= 0.0)
            continue;

        const QSizeF effectiveSz = QSizeF(_scaledMax[idx].size()) * _currentScale[idx];
        painter.save();
        painter.setOpacity(drawOpacity);
        painter.translate((width() - effectiveSz.width()) / 2.0, (height() - effectiveSz.height()) / 2.0);
        painter.scale(_currentScale[idx], _currentScale[idx]);
        painter.drawPixmap(0, 0, _scaledMax[idx]);
        painter.restore();
    }
}

void BannerCarouselWidget::onZoom0Changed(const QVariant &value)
{
    if (_scaledMax[0].isNull())
        return;
    _painterPath.clear();
    _painterPath.addRoundedRect(rect(), 0, 0);
    _currentScale[0] = value.toReal() / kZoomEnd;
    update();
}

void BannerCarouselWidget::onZoom1Changed(const QVariant &value)
{
    if (_scaledMax[1].isNull())
        return;
    _painterPath.clear();
    _painterPath.addRoundedRect(rect(), 0, 0);
    _currentScale[1] = value.toReal() / kZoomEnd;
    update();
}

void BannerCarouselWidget::onOpacityChanged(const QVariant &value)
{
    _opacity = value.toReal();
    update();
}

void BannerCarouselWidget::onOpacityFinished()
{
    _fadeOutIdx = -1;
    _opacity = 1.0;
    update();
}

// 仅重置当前图动画 — resize 专用，不触发 nextImageNeeded
void BannerCarouselWidget::restartCurrentAnimation()
{
    if (_pixmaps[_current].isNull())
        return;
    _scaledMax[_current] = _pixmaps[_current].scaled(
        size() * kZoomEnd,
        Qt::AspectRatioMode::KeepAspectRatioByExpanding,
        Qt::TransformationMode::SmoothTransformation
    );
    _zoomAni[_current]->start();
    _fadeInOutTimer.start();
}

// 完整轮播启动 — 重置动画 + 通知外部预加载下一张
void BannerCarouselWidget::startZoomAnimation()
{
    restartCurrentAnimation();
    emit nextImageNeeded();
}

void BannerCarouselWidget::onFadeOutStart()
{
    // 备用槽位无图则停在当前图，仅重启放大动画
    if (_pixmaps[1 - _current].isNull())
    {
        startZoomAnimation();
        return;
    }
    _fadeOutIdx = _current;
    _current = !_current;
    startZoomAnimation(); // 新图以 opacity=1 显示（内部会 emit nextImageNeeded）
    _opacityAni->start(); // 旧图开始 1→0 淡出
}

//
// Created by TX on 2026/6/17.
//

#include "WaveWidget.h"
#include <QPainter>
#include <QPainterPath>

#include "Core/SizeManager/SizeManager.h"
#include "Core/Theme/ThemeModeController.h"


constexpr int SamplingAccuracy = 1; // 采样间隔(px)，1=最精细，值越大越粗糙
constexpr qreal PI_4 = 4 * M_PI; // 4π，用于计算正弦周期 (2π / (4π/2w) = w)

// ─────────────────────────────────────────────
// 4 层波浪参数表（下标 0~3 对应 Layer 1~4）
// 参考 Mizuki 博客 SVG: viewBox="0 20 150 32"
//   第1层 y-offset=0  opacity=0.25  duration=7s   delay=-2s
//   第2层 y-offset=3  opacity=0.50  duration=10s  delay=-3s
//   第3层 y-offset=5  opacity=0.75  duration=13s  delay=-4s
//   第4层 y-offset=7  opacity=1.0   duration=20s  delay=-5s
// ─────────────────────────────────────────────
constexpr qreal Duration[] = {7000.0, 10000.0, 13000.0, 20000.0}; // 动画周期(ms)
constexpr int Delay[] = {2000, 3000, 4000, 5000}; // 负延迟(ms)，模拟 CSS animation-delay
constexpr qreal OffsetRatio[] = {0.0, 3.0 / 32.0, 5.0 / 32.0, 7.0 / 32.0}; // y 偏移 / viewBox 高度(32)
constexpr qreal RemainingRatio[] = {1.0, 29.0 / 32.0, 27.0 / 32.0, 25.0 / 32.0}; // 剩余高度占比 = 1 - offsetRatio
constexpr qreal Alpha[] = {0.25, 0.50, 0.75, 1.0}; // 透明度


WaveWidget::WaveWidget(QWidget *parent) :
    QWidget(parent)
{
#ifdef Q_OS_WIN
    _tickTimer.setInterval(17); // ~58fps
#endif
#ifdef __EMSCRIPTEN__
    _tickTimer.setInterval(30); // ~33fps，平衡流畅度与 WASM 性能
#endif

    connect(&_tickTimer, &QTimer::timeout, this, &WaveWidget::recalcOffsets);

    // _debounceTimer.setSingleShot(true);
    // _debounceTimer.setInterval(100); // resize 停止后等 100ms 再重建，避免连续 resize 反复渲
    // connect(&_debounceTimer, &QTimer::timeout, this, &WaveWidget::restartCurrentAnimation);
    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &WaveWidget::restartCurrentAnimation);

    updateFillColors();
    connect(&ThemeModeController::controller(), &ThemeModeController::appThemeChange, this, &WaveWidget::updateFillColors);
}

// =============================================================================
// 颜色更新 — 主题切换时调用
// =============================================================================
// Light: #E9F0F5 Dark: #080E13
void WaveWidget::updateFillColors()
{
    const bool light = ThemeModeController::controller().isAppLight();
    const int r = light ? 0xE9 : 0x08;
    const int g = light ? 0xF0 : 0x0E;
    const int b = light ? 0xF5 : 0x13;

    for (int i = 0; i < 4; ++i)
        _colors[i] = QColor(r, g, b, qRound(255 * Alpha[i]));

    renderLayer(); // 重渲 Pixmap
    update();
}

// =============================================================================
// 单层路径构建 → Pixmap 渲染
// =============================================================================
//   offsetRatio  : 波峰起点占 widget 高度的比例（如 0.21875 = 7/32）
//   remainingRatio: 波谷到 widget 底部的剩余比例，决定振幅
//   波峰 y = h * offsetRatio, 波谷 y = h, 正弦中心 = (波峰+波谷)/2
void WaveWidget::pathsToPixmap(const qreal offsetRatio, const qreal remainingRatio, const QColor &color, QPixmap &pixmap) const
{
    const int w = width();
    const int h = height();
    const int w_2 = w * 2; // 路径宽度 = 2 倍可视宽度，包含两个正弦周期

    // ── 构建 QPainterPath ──
    QPainterPath path;
    const int amplitude = qRound(h * remainingRatio / 2); // 正弦半振幅
    const int yOffset = qRound(h * offsetRatio + amplitude); // 正弦中心线 y 坐标

    path.moveTo(0, amplitude); // 起点：左边缘在振幅高度
    for (int i = SamplingAccuracy; i <= w_2; i += SamplingAccuracy)
        path.lineTo(i, amplitude * qSin(PI_4 / w_2 * i) + yOffset);

    // sin 周期: 2π / (4π/2w) = w，路径宽度 2w 恰好包含两个完整周期
    path.lineTo(w_2, h); // 右下角
    path.lineTo(0, h); // 左下角（充填区域底部）
    path.closeSubpath(); // 闭合回到起点

    // ── 渲染到透明底 QPixmap ──
    pixmap = QPixmap(w_2, h);
    pixmap.fill(Qt::GlobalColor::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::RenderHint::Antialiasing); // 渲染时抗锯齿，后续 drawPixmap 跳过
    p.fillPath(path, color); // 含 alpha 的颜色直接填充
}

void WaveWidget::renderLayer()
{
    for (int i = 0; i < 4; ++i) // 4 层全部渲染到 Pixmap
        pathsToPixmap(OffsetRatio[i], RemainingRatio[i], _colors[i], _pixmaps[i]);
}

// =============================================================================
// 每 tick 更新水平偏移 — 由 _tickTimer 触发
// =============================================================================
// offset = -w + w * ((t + delay) / duration % 1)   始终在 [-w, 0]
// fmod 取小数部分实现自动循环，delay 模拟 CSS animation-delay（负延迟）
void WaveWidget::recalcOffsets()
{
    const qint64 t = _elapsed.elapsed(); // 动画已运行毫秒数
    const int w = width();
    for (int i = 0; i < 4; ++i)
        _offsets[i] = qRound(-w + w * std::fmod((t + Delay[i]) / Duration[i], 1.0));
    update();
}

void WaveWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    for (int i = 0; i < 4; ++i)
        painter.drawPixmap(_offsets[i], 0, _pixmaps[i]);
}

void WaveWidget::restartCurrentAnimation()
{
    _tickTimer.stop(); // 停止动画
    const int vh = qRound(window()->height() / window()->devicePixelRatio());
    const int vw = qRound(window()->width() / window()->devicePixelRatio());
    const qreal ratio = vw >= 1280 ? 0.089 : vw >= 768 ? 0.072 : vw >= 480 ? 0.059 : 0.0476; // 第4层波浪占比视口高度百分比
    const int waveH = qRound(qBound(50.f, vh * ratio, 150.f) / 0.78125); // 第4层占比整个波浪区域高度的0.78125，计算得到整个波浪区域高度
    setFixedHeight(waveH);
    renderLayer(); // 重新构建 4 层路径 + Pixmap
    _elapsed.start(); // 重置时钟
    recalcOffsets(); // 立即算初始偏移，避免首帧空白
    _tickTimer.start(); // 启动定时器驱动
}

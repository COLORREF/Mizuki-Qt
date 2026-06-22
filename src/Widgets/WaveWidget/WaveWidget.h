//
// Created by TX on 2026/6/17.
//

#ifndef GITHUBPAGE_WAVEWIDGET_H
#define GITHUBPAGE_WAVEWIDGET_H
#include <QElapsedTimer>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

/*
 * 波浪动画组件 — 4 层正弦波叠加，模拟水波纹效果
 *
 * 参考 Mizuki 博客 (Astro + SVG)，用 Qt/C++ 在 WASM 上重新实现：
 *   - 1 个 QTimer 驱动全部 4 层（替代 4 个 QVariantAnimation）
 *   - 路径 → Pixmap 预渲染缓存，paintEvent 只做纹理 blit
 *   - 相位差通过 elapsed + delay 在时间域表达，sin 公式无额外偏移
 */
class WaveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override; // 4 次 drawPixmap，无抗锯齿无 fillPath
    void resizeEvent(QResizeEvent *event) override; // 停止驱动，debounce 后重建

private slots:
    void restartCurrentAnimation(); // 重建路径+Pixmap，重启时钟

private:
    void updateFillColors(); // 主题切换时更新颜色并重渲染 Pixmap
    void recalcOffsets(); // 每 tick 计算 4 层水平偏移，调用 update()
    void pathsToPixmap(qreal offsetRatio, qreal remainingRatio, const QColor &color, QPixmap &pixmap) const; // 构建单层路径 → Pixmap
    void renderLayer(); // 4 层一起构建 → Pixmap 缓存

    QPixmap _pixmaps[4]; // Pixmap 缓存 [L1..L4]，尺寸 2w×h
    int _offsets[4] = {0, 0, 0, 0}; // 水平偏移 [-w, 0]，控制 scroll 视觉位置
    QColor _colors[4]; // 填充色（RGB+Alpha），随主题切换更新

    QTimer _tickTimer; // 28ms 定时器驱动动画 (~35fps)
    QElapsedTimer _elapsed; // 单调递增计时器，动画的统一时间源
    QTimer _debounceTimer; // resize 去抖 (100ms singleShot)
};

#endif //GITHUBPAGE_WAVEWIDGET_H

//
// Created by TX on 2026/6/19.
//

#ifndef GITHUBPAGE_TYPEWRITERLABEL_H
#define GITHUBPAGE_TYPEWRITERLABEL_H
#include <QLabel>
#include <QStringList>
#include <QTimer>

/*
 * 打字机效果标签 — QLabel 子类，QLabel + QTimer 实现轮播打字机动画
 *
 * 参考 Mizuki 博客 (Astro TypewriterText)，用 Qt/C++ 在 WASM 上重新实现：
 *   - 状态机：Type(打字) → Pause(暂停) → Delete(删除) → Type(下一条)
 *   - 单一 QTimer，根据状态切换 interval (typeMs / pauseMs / deleteMs)
 *   - setFixedWidth 锁定最长文本宽度，消除文本长度变化导致的布局抖动
 *   - setTexts() 支持任意数量文本，单条文本打完即停不删除
 */
class TypewriterLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TypewriterLabel(QWidget *parent = nullptr);

    /// 设置轮播文本列表；自动计算 max-width 防抖
    void setTexts(const QStringList &texts);

    /// typeMs=100, deleteMs=50, pauseMs=2000（同 Mizuki 博客参数）
    void setSpeed(int typeMs, int deleteMs, int pauseMs);

public slots:
    void start(); // 开始/继续动画
    void stop(); // 停止动画，保留当前显示
    void skip(); // 立即跳到下一条文本

private slots:
    void tick(); // QTimer 回调，驱动状态机
    void reSetFontSize();

private:
    enum class State { Type, Pause, Delete };

    QTimer _timer;
    QStringList _texts;
    State _state = State::Type;
    int _textIdx = 0;
    int _charIdx = 0;
    int _typeMs = 100;
    int _deleteMs = 50;
    int _pauseMs = 2000;
};

#endif // GITHUBPAGE_TYPEWRITERLABEL_H

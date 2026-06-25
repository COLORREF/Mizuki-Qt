#ifndef SYSTEMTHEMEMONITOR_H
#define SYSTEMTHEMEMONITOR_H

#include <QObject>
#include "Core/Defs.h"

#ifdef Q_OS_WIN
#include <QColor>
#include <QMap>
#include <QThread>
#include <Windows.h>

#endif


// ── 系统主题监听器 ──
//     WASM:  基于 matchMedia('prefers-color-scheme: dark') 事件，无需线程
//     Win:   基于注册表 RegNotifyChangeKeyValue + QThread 阻塞等待
class SystemThemeMonitor final :
#ifdef Q_OS_WIN
        public QThread
#elif defined(__EMSCRIPTEN__)
        public QObject
#endif
{
    Q_OBJECT

    explicit SystemThemeMonitor(QObject *parent = nullptr);

public:
    static SystemThemeMonitor &monitor();

    [[nodiscard]] static Theme SystemTheme();

    void start();

    void stop();

signals:
    void systemThemeChanged(Theme theme);

#ifdef Q_OS_WIN
    void systemAccentColorsChanged(QMap<SysAccentPalette, QColor> colors);

protected:
    void run() override;

private:
    HKEY _regs[2] = {nullptr}; // 注册表项句柄
    HANDLE _hEvent_array[3] = {nullptr}; // 事件对象数组

#elif defined(__EMSCRIPTEN__)
public:
    // JS → C++ 桥接入口，仅由 _onSystemThemeChange 回调调用
    void onJsThemeChange(bool isDark);

private:
    bool _listening = false;
#endif
};

#endif // SYSTEMTHEMEMONITOR_H

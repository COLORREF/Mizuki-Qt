#ifndef SYSTEMTHEMEMONITOR_H
#define SYSTEMTHEMEMONITOR_H

#include <QObject>
#include "Core/Defs.h"

#ifdef Q_OS_WIN
#include <QColor>
#include <QMap>
#include <QThread>
#include <Windows.h>

/**
 * 系统主题色及其衍生色，命名采用与 WinRT 一致的方式
 * 值从小到大的顺序对应颜色由浅到深
 * ！不要修改顺序，sysAccentColors 实现函数依赖此顺序
 */
enum class SysAccentPalette
{
    AccentLight3, // 最浅的主题色  场景：1、深色模式下鼠标悬浮在控件（如右下角面板打开后的WiFi按钮）上时使用的颜色 2、深色模式下系统超链接标签文字颜色
    AccentLight2, // 较浅的主题色  场景：1、深色模式下系统控件的默认颜色（如系统设置界面侧边栏指示器的颜色、单选框内部背景色）2、浅色模式下系统超链接标签文字颜色
    AccentLight1, // 浅色主题色
    Accent,       // 个性化设置中实际设置显示的颜色
    AccentDark1,  // 深色主题色   场景：1、浅色模式下系统控件的默认颜色
    AccentDark2,  // 较深的主题色  场景：1、系统底部任务栏颜色（需要开启深色模式以及"在开始和任务栏显示重点颜色"）
                  //              2、浅色模式下鼠标悬浮在控件（如右下角面板打开后的WiFi按钮）上时使用的颜色
                  //              3、浅色模式下用于突出重要内容的超链接标签颜色（如任务管理器设置中相关超链接标签）
    AccentDark3,  // 最深的主题色  场景：深色模式下系统列表控件选中时的颜色（如任务管理器中内存或cpu消耗占比较高的单元格颜色与之非常接近）
};
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

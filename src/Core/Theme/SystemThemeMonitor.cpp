#include "SystemThemeMonitor.h"

// ══════════════════════════════════════════════════════════════
//  WASM 平台实现
// ══════════════════════════════════════════════════════════════
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// JS → C++ 桥接
//   EMSCRIPTEN_KEEPALIVE 防止链接器优化掉；extern "C" 阻止 C++ 名称修饰
//   浏览器 matchMedia 事件回调 → 调用 monitor().onJsThemeChange()
//   （函数名不要以 _ 开头，否则 Emscripten 导出为 Module.__xxx）
extern "C" EMSCRIPTEN_KEEPALIVE
void onSystemThemeChange(int isDark)
{
    SystemThemeMonitor::monitor().onJsThemeChange(isDark != 0);
}

// ── 单例 ──
SystemThemeMonitor &SystemThemeMonitor::monitor()
{
    static auto *instance = new SystemThemeMonitor;
    return *instance;
}

SystemThemeMonitor::SystemThemeMonitor(QObject *parent) :
    QObject(parent) {}

// ── 同步查询当前系统主题（通过 matchMedia 立即返回） ──
Theme SystemThemeMonitor::SystemTheme()
{
    const int isDark = EM_ASM_INT({return window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;});
    return isDark ? Theme::Dark : Theme::Light;
}

// ── 注册 matchMedia 事件监听 ──
void SystemThemeMonitor::start()
{
    if (_listening)
        return;
    _listening = true;

    EM_ASM({
            console.log('[SystemThemeMonitor] 已注册 matchMedia 监听');
            Module._systemDarkQuery = window.matchMedia('(prefers-color-scheme: dark)');
            Module._systemDarkQuery.addEventListener('change', function(e) {
                console.log('[SystemThemeMonitor] 系统主题变化: ' + (e.matches ? '深色(dark)' : '浅色(light)'));
                Module._onSystemThemeChange(e.matches ? 1 : 0);
            });
        }
    );
}

// ── 停止监听（页面卸载时自动清理） ──
void SystemThemeMonitor::stop()
{
    _listening = false;
}

// ── JS 回调入口 ──
void SystemThemeMonitor::onJsThemeChange(bool isDark)
{
    emit systemThemeChanged(isDark ? Theme::Dark : Theme::Light);
}

#endif // __EMSCRIPTEN__


// ══════════════════════════════════════════════════════════════
//  Windows 平台实现
// ══════════════════════════════════════════════════════════════
#ifdef Q_OS_WIN

#include <QApplication>

// ── 注册表路径常量 ──
//   Personalize\AppsUseLightTheme:  DWORD, 0=深色 1=浅色
#define THEME_REGEDIT_PATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize")
#define THEME_REGEDIT_KEY TEXT("AppsUseLightTheme")
#define ACCENT_PALETTE_REGEDIT_PATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent")
#define ACCENT_PALETTE_KEY TEXT("AccentPalette")

// ── 单例 ──
SystemThemeMonitor &SystemThemeMonitor::monitor()
{
    static auto *instance = new SystemThemeMonitor(qApp);
    return *instance;
}

SystemThemeMonitor::SystemThemeMonitor(QObject *parent) :
    QThread(parent) {}

// ── 同步查询当前系统主题（读注册表） ──
Theme SystemThemeMonitor::SystemTheme()
{
    HKEY reg;
    ::RegOpenKeyEx(HKEY_CURRENT_USER, THEME_REGEDIT_PATH, 0,KEY_NOTIFY | KEY_READ, &reg);
    // 获取深浅主题改变前注册表项值
    DWORD dw_size = sizeof(DWORD);
    DWORD theme;
    ::RegQueryValueEx(reg, THEME_REGEDIT_KEY, nullptr, nullptr, reinterpret_cast<LPBYTE>(&theme), &dw_size);
    RegCloseKey(reg);
    return theme ? Theme::Light : Theme::Dark;
}

// ── 启动监听线程 ──
void SystemThemeMonitor::start()
{
    if (!isRunning())
        QThread::start();
}

// ── 停止监听线程 ──
void SystemThemeMonitor::stop()
{
    ::SetEvent(this->_hEvent_array[2]);
}

// ── 线程入口：注册表变化轮询 ──
void SystemThemeMonitor::run()
{
    // 打开深浅主题注册表项
    ::RegOpenKeyEx(
        HKEY_CURRENT_USER,
        THEME_REGEDIT_PATH,
        0,
        KEY_NOTIFY | KEY_READ,
        &this->_regs[0]
    );

    //打开主题配色注册表
    ::RegOpenKeyEx(
        HKEY_CURRENT_USER,
        ACCENT_PALETTE_REGEDIT_PATH,
        0,
        KEY_NOTIFY | KEY_READ,
        &this->_regs[1]
    );

    // 创建事件对象
    this->_hEvent_array[0] = ::CreateEvent(nullptr, TRUE, FALSE, nullptr); //themeChange
    this->_hEvent_array[1] = ::CreateEvent(nullptr, TRUE, FALSE, nullptr); //accentColorChange
    this->_hEvent_array[2] = ::CreateEvent(nullptr, TRUE, FALSE, nullptr); //exit

    // 获取深浅主题改变前注册表项值
    DWORD dw_size = sizeof(DWORD);
    DWORD last_theme;
    DWORD current_theme;
    ::RegQueryValueEx(this->_regs[0],
                      THEME_REGEDIT_KEY,
                      nullptr,
                      nullptr,
                      reinterpret_cast<LPBYTE>(&last_theme),
                      &dw_size
    );

    // 获取系统配色改变前的注册表项值
    const auto color_analysis = [](BYTE data[])-> QMap<SysAccentPalette, QColor> {
        QMap<SysAccentPalette, QColor> colors;
        for (int i = 0; i < 7; ++i) // 解析前 7 个颜色（每 4 字节一个颜色，RGBA 顺序） int alpha = data[i * 4 + 3]
            colors[static_cast<SysAccentPalette>(i)] = {data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2], 255};
        return colors;
    };
    BYTE accent_colors_data[32];
    DWORD colors_size = sizeof(accent_colors_data);
    QMap<SysAccentPalette, QColor> last_accent_colors = color_analysis(accent_colors_data);
    RegQueryValueEx(this->_regs[1],
                    ACCENT_PALETTE_KEY,
                    nullptr,
                    nullptr,
                    accent_colors_data,
                    &colors_size
    );

    while (true)
    {
        // 监测注册表项值，让WaitForMultipleObjects等待此事件
        ::RegNotifyChangeKeyValue(this->_regs[0], TRUE, REG_NOTIFY_CHANGE_LAST_SET, this->_hEvent_array[0], TRUE);
        ::RegNotifyChangeKeyValue(this->_regs[1], TRUE, REG_NOTIFY_CHANGE_LAST_SET, this->_hEvent_array[1], TRUE);

        // 阻塞等待
        const DWORD dwWait = ::WaitForMultipleObjects(3, this->_hEvent_array, FALSE, INFINITE);

        // 深浅主题变化
        if (dwWait == WAIT_OBJECT_0)
        {
            // 获取当前值
            ::RegQueryValueEx(this->_regs[0],
                              THEME_REGEDIT_KEY,
                              nullptr,
                              nullptr,
                              reinterpret_cast<LPBYTE>(&current_theme),
                              &dw_size
            );
            if (last_theme != current_theme)
            {
                if (current_theme)
                    emit this->systemThemeChanged(Theme::Light);
                else
                    emit this->systemThemeChanged(Theme::Dark);
                last_theme = current_theme; // 0为深色，1为浅色
            }
            ::ResetEvent(this->_hEvent_array[0]); // 重置事件对象无事件
            continue;
        }

        // 主题配色变化
        if (dwWait == WAIT_OBJECT_0 + 1)
        {
            RegQueryValueEx(this->_regs[1],
                            ACCENT_PALETTE_KEY,
                            nullptr,
                            nullptr,
                            accent_colors_data,
                            &colors_size
            );
            QMap<SysAccentPalette, QColor> current_accent_colors = color_analysis(accent_colors_data);
            if (last_accent_colors != current_accent_colors)
            {
                emit systemAccentColorsChanged(current_accent_colors);
                last_accent_colors = current_accent_colors;
            }
            ::ResetEvent(this->_hEvent_array[1]);
            continue;
        }

        // 退出线程
        if (dwWait == WAIT_OBJECT_0 + 2)
        {
            for (const auto &hEvent: this->_hEvent_array)
            {
                ::ResetEvent(hEvent);
                ::CloseHandle(hEvent);
            }
            for (const auto &reg: this->_regs)
                ::RegCloseKey(reg);
            return;
        }
    }
}

#endif // Q_OS_WIN

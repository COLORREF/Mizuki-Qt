#include "ThemeModeController.h"
#include "SystemThemeMonitor.h"

#include <QApplication>

// ── 构造：查询当前系统主题 → 应用初始跟随 → 启动监听 ──
ThemeModeController::ThemeModeController(QObject *parent) :
    QObject(parent)
{
    _system_theme = SystemThemeMonitor::SystemTheme(); // 同步查询一次
    _app_theme = _system_theme; // 初始应用主题 = 系统主题
    SystemThemeMonitor::monitor().setParent(this);
    SystemThemeMonitor::monitor().start(); // 启动 matchMedia 事件监听
    connect(&SystemThemeMonitor::monitor(), &SystemThemeMonitor::systemThemeChanged, this, &ThemeModeController::onSystemThemeChanged);
}

// ── 单例 ──
ThemeModeController &ThemeModeController::controller()
{
    static auto *instance = new ThemeModeController(qApp);
    return *instance;
}

// ── 析构：停止监听 ──
ThemeModeController::~ThemeModeController()
{
#ifdef Q_OS_WIN
    if (SystemThemeMonitor::monitor().isRunning())
        SystemThemeMonitor::monitor().stop();
    SystemThemeMonitor::monitor().wait();
#endif
#ifdef __EMSCRIPTEN__
    SystemThemeMonitor::monitor().stop();
#endif
}

// ── 手动设置应用主题（脱离跟随系统模式） ──
void ThemeModeController::setAppTheme(const Theme theme)
{
    _is_follow_system = false;
    _app_theme = theme;
    emit appThemeChange(theme);
}

// ── 仅当前为深色时才切到浅色 ──
void ThemeModeController::toggleAppLight()
{
    if (_app_theme == Theme::Dark)
        setAppTheme(Theme::Light);
}

// ── 仅当前为浅色时才切到深色 ──
void ThemeModeController::toggleAppDark()
{
    if (_app_theme == Theme::Light)
        setAppTheme(Theme::Dark);
}

// ── 恢复跟随系统主题（应用主题设为系统当前主题） ──
void ThemeModeController::followSystem()
{
    if (!_is_follow_system) // 避免重复触发
    {
        _is_follow_system = true;
        _app_theme = _system_theme;
        emit appThemeChange(_app_theme);
    }
}

// ── 深浅切换（Light ↔ Dark） ──
void ThemeModeController::toggleAppTheme()
{
    setAppTheme(_app_theme == Theme::Light ? Theme::Dark : Theme::Light);
}

// ── 系统主题变化时的内部处理 ──
void ThemeModeController::onSystemThemeChanged(const Theme theme)
{
    _system_theme = theme;
    emit systemThemeChange(theme); // 通知"系统主题变了"

    if (_is_follow_system) // 跟随模式下同步应用主题
    {
        _app_theme = _system_theme;
        emit appThemeChange(_app_theme);
    }
}


// ── 查询方法 ──
Theme ThemeModeController::appTheme() const { return _app_theme; }
Theme ThemeModeController::systemTheme() const { return _system_theme; }
bool ThemeModeController::isAppLight() const { return _app_theme == Theme::Light; }
bool ThemeModeController::isAppDark() const { return _app_theme == Theme::Dark; }
bool ThemeModeController::isFollowSystem() const { return _is_follow_system; }

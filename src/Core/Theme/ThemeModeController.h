#ifndef THEMEMODECONTROLLER_H
#define THEMEMODECONTROLLER_H

#include <QObject>
#include "Theme.h"

class ThemeModeController final : public QObject
{
    Q_OBJECT
    friend class SystemThemeMonitor;

    explicit ThemeModeController(QObject *parent = nullptr);

public:
    ~ThemeModeController() override;

    static ThemeModeController &controller();

    [[nodiscard]] Theme appTheme() const;

    [[nodiscard]] Theme systemTheme() const;

    [[nodiscard]] bool isAppLight() const;

    [[nodiscard]] bool isAppDark() const;

    [[nodiscard]] bool isFollowSystem() const;

public slots:
    // ── 控制 ──
    void setAppTheme(Theme theme);

    void toggleAppLight();

    void toggleAppDark();

    void followSystem();

    void toggleAppTheme();

signals:
    void appThemeChange(Theme theme);

    void systemThemeChange(Theme theme);

private slots:
    void onSystemThemeChanged(Theme theme);

private:
    Theme _app_theme;
    Theme _system_theme;
    bool _is_follow_system = true;
};

#endif // THEMEMODECONTROLLER_H

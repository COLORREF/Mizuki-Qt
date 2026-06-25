//
// Created by TX on 2026/6/25.
//

#ifndef MIZUKI_QT_PALETTE_H
#define MIZUKI_QT_PALETTE_H

#include <QColor>
#include <QHash>

#include "Core/Defs.h"

/**
 * 全局调色板 — 单例，管理 Light/Dark 两套颜色方案
 *
 * - JSON（唯一色彩来源）存储 oklch(L,C) 参数与固定色 hex
 * - Default 组：从 JSON 加载参数，hue=240° 实时计算 sRGB
 * - Custom 组：用户指定色相（0~360°），基于同一套 oklch(L,C) 参数计算
 * - 随 ThemeModeController::appThemeChange 自动切换当前主题
 */
class Palette final : public QObject
{
    Q_OBJECT

public:
    using ColorsHash = QHash<ColorRole, QColor>;
    using ColorGroup = QPair<ColorGroups, Theme>;

    explicit Palette(QObject *parent = nullptr);

    ~Palette() override = default;

    /// 全局单例
    static Palette &instance();

    /// 获取当前配色组下的颜色
    [[nodiscard]] QColor color(ColorRole role) const;

    /// 获取指定配色组 + 主题下的颜色
    [[nodiscard]] QColor color(ColorRole role, Theme theme) const;

    /// 等价于 color(ColorRole)
    [[nodiscard]] QColor operator[](ColorRole role) const;

    /// 当前配色组
    [[nodiscard]] ColorGroup groupTheme() const { return _groupTheme; }

    /**
     * @brief 设置使用的颜色组策略
     * @param group 将要设置的颜色组
     * @param hue  自定义色相（0~360°）
     *              - 仅当 group == ColorGroups::Custom 时 hue 参数生效
     */
    void setColorGroups(ColorGroups group, int hue = 240);

    /**
     * @brief 设置自定义色相
     * @param hue 色相角度（0~360°）
     *            - 仅当颜色组为 Custom 时函数生效
     *            - 亮度和饱和度使用与前端参考网页一致的默认值
     */
    void setCustomColor(int hue);

signals:
    /// 主题切换、配色组变更或自定义色相更新后发射
    void appColorChange();

private slots:
    void onAppThemeChanged(Theme theme);

private:
    /// 每个 ColorRole 的 oklch(L,C) 参数，从 JSON 加载
    struct OklchRoleParams
    {
        double lightL = 0, lightC = 0;
        double darkL = 0, darkC = 0;
        bool isFixed = false; // 固定色：不参与 generateCustomScheme 的 hue 旋转
    };

    /// 根据 oklch(L,C) + hue 计算 sRGB 颜色
    static QColor oklchToColor(double l, double c, double hDeg);

    void loadSchemes();

    /// 根据 hue 值生成一套完整配色（浅色 + 深色），写入 _schemes[Custom, theme]
    void generateCustomScheme(int hue);

    /// 将 JSON hex 字符串（"#AABBCC" / "#AABBCCDD"）解析为 QColor
    static QColor parseColorFromJsonStr(const QString &raw);

    ColorGroup _groupTheme;
    QHash<ColorGroup, ColorsHash> _schemes;
    QHash<ColorRole, OklchRoleParams> _oklchParams;
};

#endif //MIZUKI_QT_PALETTE_H

//
// Created by TX on 2026/6/25.
//

#ifndef MIZUKI_QT_DEFS_H
#define MIZUKI_QT_DEFS_H

#include <QObject>

enum class Theme
{
    Light,
    Dark
};

enum class ColorGroups
{
    Default, // 默认配色方案：采用色相 240°（蓝），与前端参考网页一致
    Custom // 自定义配色方案：由用户指定色相（0~360°），亮度和饱和度保持默认
};

enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

/**
 * 颜色角色 — 语义对齐参考网页 Mizuki 博客的 CSS 变量体系
 *
 * 每个角色在浅色 / 深色模式下各有一组独立的 oklch(L C H) 参数，
 * L（亮度）和 C（饱和度）由设计固定，H（色相）可随调色盘滑块变化。
 * 默认 H = 240°（蓝），对应参考网页的 --configHue: 240。
 */

namespace ColorRoles
{
    Q_NAMESPACE

    enum class ColorRole
    {
        Primary,
        PageBg,
        CardBg,
        CardBgTransparent,
        FloatPanelBg,
        DeepText,
        TitleActive,
        BtnContent,
        BtnRegularBg,
        BtnRegularBgHover,
        BtnRegularBgActive,
        BtnPlainBgHover,
        BtnPlainBgActive,
        BtnCardBgHover,
        BtnCardBgActive,
        LineDivider,
        LineColor,
        ContentMeta,
        SelectionBg,
        LinkUnderline,
        ScrollbarBg,

        Last
    };

    Q_ENUM_NS(ColorRole)
}

using ColorRoles::ColorRole;

#endif //MIZUKI_QT_DEFS_H

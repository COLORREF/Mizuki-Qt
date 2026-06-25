//
// Created by TX on 2026/6/25.
//

#ifndef MIZUKI_QT_RESOURCEPATH_H
#define MIZUKI_QT_RESOURCEPATH_H

#include <QString>
#include <QStringList>

// ============================================================
// 资源路径统一声明
//
// 所有后端 URL、本地 qrc 资源路径均集中在此一处维护，
// 新增 / 修改资源路径只需改此文件，无需在业务代码中搜索硬编码字符串。
// ============================================================

namespace ResourcePath
{
    // ─────────────────────────────────────────────
    // 后端服务器相对路径（WASM 下浏览器自动解析 origin，Windows 下加 serverBase() 前缀）
    // ─────────────────────────────────────────────
    namespace Server
    {
        // 字体
        inline const QStringList Fonts = {
            QStringLiteral("font/DreamHanSansCN/DreamHanSansCN-W9.ttf"),
            QStringLiteral("font/ZenMaruGothic-Medium.ttf"),
        };

        // 图片子目录名
        inline constexpr auto CategoryHomepage = "Homepage";
        inline constexpr auto CategoryProfile = "Profile";

        // API 端点
        inline QString apiMaxImageId(const QString &category)
        {
            return QStringLiteral("api/images/max_id/%1/").arg(category);
        }

        // 静态图片 URL
        inline QString imageUrl(const QString &category, int id, const QString &ext = "webp")
        {
            return QStringLiteral("Images/%1/%2.%3").arg(category).arg(id).arg(ext);
        }

        inline QString avatarUrl()
        {
            return QStringLiteral("Images/Profile/avatar.jpg");
        }
    }

    // ─────────────────────────────────────────────
    // 本地 Qt 资源（qrc 编译嵌入，通过 :/ 前缀访问）
    // ─────────────────────────────────────────────
    namespace Local
    {
        // 主页兜底图片（1.webp / 2.webp 交替）
        inline QString fallbackHomeImage(int idx)
        {
            return QStringLiteral(":/image/%1.webp").arg(idx);
        }

        // 头像兜底图片
        inline constexpr auto FallbackAvatar = ":/image/avatar.jpg";

        // 调色板 JSON 配置
        inline constexpr auto ColorProfile = ":/Profiles/Palette/ColorProfile.json";
    }

    // ─────────────────────────────────────────────
    // 平台基地址
    // ─────────────────────────────────────────────
#ifdef Q_OS_WIN
    inline QString serverBase() { return QStringLiteral("http://localhost:8000/"); }
#else
    inline QString serverBase() { return QString(); }
#endif
} // namespace ResourcePath

#endif //MIZUKI_QT_RESOURCEPATH_H

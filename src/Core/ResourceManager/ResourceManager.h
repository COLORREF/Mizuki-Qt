#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPixmap>

class QNetworkAccessManager;

// ============================================================
// 资源管理器 (全局单例) — 负责从服务器异步下载各类资源
// ============================================================
// 加载策略分两种：
//   批量(Eager)  — 字体
//                 程序启动时一次性全部拉取，就绪后 emit all*Loaded()
//   按需(Lazy)   — 图片
//                 loadHomeImage 内部自行获取 max_id 后再请求图片
//                 WASM 场景下浏览器自动处理 HTTP 缓存，无需应用层再次缓存
// ============================================================
class ResourceManager : public QObject
{
    Q_OBJECT

public:
    static ResourceManager &manager();

    ResourceManager(const ResourceManager &) = delete;

    ResourceManager &operator=(const ResourceManager &) = delete;

    void loadFonts(); // 异步下载注册梦源黑体 (W5/W7/W9) + loli 体 + Zen Maru Gothic

public slots:
    void loadHomeImage(); // 异步顺序请求一张主页图片（1 → max → 1 循环）
    void loadAvatar();    // 异步请求头像图片，失败时自动回退到本地资源

signals:
    void allFontsLoaded(); // 所有字体加载就绪
    void homeImageReady(QPixmap pixmap); // 主页图片加载完成
    void avatarReady(QPixmap pixmap);    // 头像加载完成（成功或回退都会发射）

private:
    explicit ResourceManager(QObject *parent = nullptr);

    void requestNextImageById(); // 顺序选号（1→max→1 循环）并发起图片请求
    void onMaxIdReplyFinished(); // max_id 请求回调
    void onImageReplyFinished(); // 图片请求回调
    void onAvatarReplyFinished(); // 头像请求回调

    QNetworkAccessManager *m_networkManager;
    int m_homepageMaxId = 0; // Homepage 最大图片编号（0 = 尚未获取 / 无图片）
    int m_lastHomeImageId = 0; // 上次请求的图片编号（0 = 尚未请求过）
    bool m_maxIdRequestInFlight = false; // 正在请求 max_id，避免并发重复请求
    int m_fallbackIdx = 0; // 兜底图片轮换索引（0↔1，对应 :/image/1.webp ↔ :/image/2.webp）
};

#endif // RESOURCEMANAGER_H

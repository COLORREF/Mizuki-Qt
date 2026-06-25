#include "ResourceManager.h"

#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>

ResourceManager &ResourceManager::manager()
{
    static auto inst = new ResourceManager(qApp);
    return *inst;
}

#ifdef Q_OS_WIN
// Windows 桌面：需要绝对 URL 指向本地 Django 服务
static const auto SERVER_BASE = QStringLiteral("http://localhost:8000/");
#else
// WASM：浏览器自动以当前页面 origin 解析相对 URL
static const QString SERVER_BASE = QString();
#endif

ResourceManager::ResourceManager(QObject *parent) :
    QObject(parent),
    m_networkManager(new QNetworkAccessManager(this)) {}

// 异步加载字体 — 每个 URL 独立发起 HTTP 请求
void ResourceManager::loadFonts()
{
    qDebug() << "开始加载字体...";
    static const QStringList FONT_URLS = {
        // QStringLiteral("font/DreamHanSansCN/DreamHanSansCN-W5.ttf"),
        // QStringLiteral("font/DreamHanSansCN/DreamHanSansCN-W7.ttf"),
        QStringLiteral("font/DreamHanSansCN/DreamHanSansCN-W9.ttf"),
        // QStringLiteral("font/loli.ttf"),
        QStringLiteral("font/ZenMaruGothic-Medium.ttf"),
    };

    const int total = static_cast<int>(FONT_URLS.size());
    auto completed = std::make_shared<int>(0);

    for (const QString &url: FONT_URLS)
    {
        QNetworkReply *reply = m_networkManager->get(QNetworkRequest(QUrl(SERVER_BASE + url)));
        connect(reply,
                &QNetworkReply::finished,
                this,
                [this, reply, url, completed, total]() {
                    if (reply->error() != QNetworkReply::NoError)
                        qWarning() << "字体下载失败:" << url << reply->errorString();
                    else
                        QFontDatabase::addApplicationFontFromData(reply->readAll());
                    reply->deleteLater();
                    (*completed)++;
                    if (*completed >= total)
                        emit allFontsLoaded();
                }
        );
    }
}

// loadHomeImage — 若 max_id 未就绪则先获取，否则直接请求图片
void ResourceManager::loadHomeImage()
{
    if (m_homepageMaxId > 0)
    {
        requestNextImageById();
        return;
    }

    if (m_maxIdRequestInFlight)
        return;
    m_maxIdRequestInFlight = true;

    const auto *reply = m_networkManager->get(QNetworkRequest(QUrl(SERVER_BASE + QStringLiteral("api/images/max_id/Homepage/"))));
    connect(reply, &QNetworkReply::finished, this, &ResourceManager::onMaxIdReplyFinished);
}

void ResourceManager::onMaxIdReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    m_maxIdRequestInFlight = false;

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "获取 Homepage 最大图片编号失败:" << reply->errorString();
        // 兜底：本地资源图片交替返回
        m_fallbackIdx = 1 - m_fallbackIdx;
        emit homeImageReady(QPixmap(QStringLiteral(":/image/%1.webp").arg(m_fallbackIdx + 1)));
        return;
    }
    const int num = reply->readAll().trimmed().toInt();
    if (num <= 0)
    {
        qWarning() << "Homepage 最大图片编号无效:" << num;
        m_fallbackIdx = 1 - m_fallbackIdx;
        emit homeImageReady(QPixmap(QStringLiteral(":/image/%1.webp").arg(m_fallbackIdx + 1)));
        return;
    }
    m_homepageMaxId = num;
    qDebug() << "Homepage 最大图片编号:" << m_homepageMaxId;
    requestNextImageById();
}

// 顺序选号 + 异步请求图片（1 → max → 1 循环）
void ResourceManager::requestNextImageById()
{
    int chosen;
    if (m_lastHomeImageId <= 0 || m_lastHomeImageId >= m_homepageMaxId)
        chosen = 1; // 首次请求或已到末尾，从头开始
    else
        chosen = m_lastHomeImageId + 1;

    m_lastHomeImageId = chosen;

    // 请求静态文件路径（图片作为静态文件提供）
    const QString url = SERVER_BASE + QStringLiteral("Images/Homepage/%1.webp").arg(chosen);
    const auto *reply = m_networkManager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, &ResourceManager::onImageReplyFinished);
}

void ResourceManager::onImageReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());

    QPixmap pixmap;
    if (reply->error() == QNetworkReply::NoError)
        pixmap.loadFromData(reply->readAll());
    else
        qWarning() << "加载主页图片失败:" << reply->url().toString();

    reply->deleteLater();
    emit homeImageReady(pixmap);
}

#include "ResourceManager.h"

#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "Core/ResourcePath.h"

ResourceManager &ResourceManager::manager()
{
    static auto inst = new ResourceManager(qApp);
    return *inst;
}

ResourceManager::ResourceManager(QObject *parent) :
    QObject(parent),
    m_networkManager(new QNetworkAccessManager(this)) {}

// 异步加载字体 — 每个 URL 独立发起 HTTP 请求
void ResourceManager::loadFonts()
{
    qDebug() << "开始加载字体...";
    const auto &fontUrls = ResourcePath::Server::Fonts;
    const int total = static_cast<int>(fontUrls.size());
    auto completed = std::make_shared<int>(0);

    for (const QString &url: fontUrls)
    {
        QNetworkReply *reply = m_networkManager->get(QNetworkRequest(QUrl(ResourcePath::serverBase() + url)));
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

    const auto url = ResourcePath::serverBase() + ResourcePath::Server::apiMaxImageId(ResourcePath::Server::CategoryHomepage);
    const auto *reply = m_networkManager->get(QNetworkRequest(QUrl(url)));
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
        m_fallbackIdx = 1 - m_fallbackIdx;
        emit homeImageReady(QPixmap(ResourcePath::Local::fallbackHomeImage(m_fallbackIdx + 1)));
        return;
    }
    const int num = reply->readAll().trimmed().toInt();
    if (num <= 0)
    {
        qWarning() << "Homepage 最大图片编号无效:" << num;
        m_fallbackIdx = 1 - m_fallbackIdx;
        emit homeImageReady(QPixmap(ResourcePath::Local::fallbackHomeImage(m_fallbackIdx + 1)));
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
        chosen = 1;
    else
        chosen = m_lastHomeImageId + 1;

    m_lastHomeImageId = chosen;

    const auto url = ResourcePath::serverBase()
                     + ResourcePath::Server::imageUrl(ResourcePath::Server::CategoryHomepage, chosen);
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

// ── 头像加载：从后端请求 → 失败回退本地资源 ──
void ResourceManager::loadAvatar()
{
    const auto url = ResourcePath::serverBase() + ResourcePath::Server::avatarUrl();
    const auto *reply = m_networkManager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, &ResourceManager::onAvatarReplyFinished);
}

void ResourceManager::onAvatarReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());

    QPixmap pixmap;
    if (reply->error() == QNetworkReply::NoError)
        pixmap.loadFromData(reply->readAll());
    else
        qWarning() << "加载头像失败，使用本地回退:" << reply->errorString();

    reply->deleteLater();

    if (pixmap.isNull())
        pixmap = QPixmap(ResourcePath::Local::FallbackAvatar);

    emit avatarReady(pixmap);
}

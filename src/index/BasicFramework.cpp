//
// Created by TX on 2026/6/12.
//

#include "BasicFramework.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include "Core/ResourceManager/ResourceManager.h"
#include "Core/Theme/ThemeModeController.h"
#include "Widgets/BannerCarouselWidget/BannerCarouselWidget.h"
#include "Widgets/TypewriterLabel/TypewriterLabel.h"
#include "Widgets/WaveWidget/WaveWidget.h"

ContentWidget::ContentWidget(QWidget *parent) :
    QWidget(parent)
{
    connect(&ThemeModeController::controller(), &ThemeModeController::appThemeChange, this, QOverload<>::of(&ContentWidget::update));
}

void ContentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::PenStyle::NoPen);
    painter.fillRect(rect(), QColor(ThemeModeController::controller().isAppLight() ? 0xE9F0F5 : 0x080E13));
}

BasicFramework::BasicFramework(QWidget *parent) :
    ScrollArea(parent),
    _vboxLayout(new QVBoxLayout(this)),
    _bannerCarousel(new BannerCarouselWidget(this)),
    _bannerVboxLayout(new QVBoxLayout(_bannerCarousel)),
    _bannerTitle(new QLabel(_bannerCarousel)),
    _typewriterLabel(new TypewriterLabel(_bannerCarousel)),
    _content(new ContentWidget(this)),
    _wave(new WaveWidget(_bannerCarousel))
{
    _debounceTimer.setSingleShot(true);
    _debounceTimer.setInterval(100);
    connect(&_debounceTimer, &QTimer::timeout, this, &BasicFramework::setSize);

    connect(_bannerCarousel, &BannerCarouselWidget::nextImageNeeded, &ResourceManager::manager(), &ResourceManager::loadHomeImage);
    connect(&ResourceManager::manager(), &ResourceManager::homeImageReady, _bannerCarousel, &BannerCarouselWidget::setPixmap);
    connect(&ResourceManager::manager(), &ResourceManager::allFontsLoaded, this, &BasicFramework::initMainUI);
    qDebug() << "开始加载字体...";
    ResourceManager::manager().loadFonts();
}

void BasicFramework::resizeEvent(QResizeEvent *event)
{
    ScrollArea::resizeEvent(event);
    _debounceTimer.start();
}

void BasicFramework::setSize() const
{
    const int vh = qRound(window()->height() / window()->devicePixelRatio());
    const int vw = qRound(window()->width() / window()->devicePixelRatio());
    const int bch = qRound(vh * 0.7);
    const qreal ratio = vw >= 1280 ? 0.089 : vw >= 768 ? 0.072 : vw >= 480 ? 0.059 : 0.0476; // 第4层波浪占比视口高度百分比
    const int waveH = qRound(qBound(50.f, vh * ratio, 150.f) / 0.78125); // 第4层占比整个波浪区域高度的0.78125，计算得到整个波浪区域高度
    _bannerCarousel->setFixedHeight(bch);
    _wave->setFixedHeight(waveH);

    // ── 响应式字体 — 对齐 Mizuki: title=96/64/53/45  sub=30/24/16/14 ──
    //    s 模拟 JS 页面缩放 clamp(0.85, clientWidth/2000, 1); vw>1280 对齐 JS innerWidth 判断
    const double s = vw > 1280 ? qBound(0.85, vw / 2000.0, 1.0) : 1.0;
    const int titlePx = vw >= 1280 ? qRound(96.0 * s) : vw >= 768 ? qRound(64.0 * s) : vw >= 480 ? 53 : 45;
    const int subPx = vw >= 1280 ? qRound(30.0 * s) : vw >= 768 ? qRound(24.0 * s) : vw >= 480 ? 16 : 14;

    QFont f1 = _typewriterLabel->font();
    f1.setPixelSize(subPx);
    _typewriterLabel->setFont(f1);

    QFont f2 = _bannerTitle->font();
    f2.setPixelSize(titlePx);
    _bannerTitle->setFont(f2);
}

void BasicFramework::initMainUI()
{
    _vboxLayout->setSpacing(0);
    _vboxLayout->setContentsMargins(0, 0, 0, 0);
    _vboxLayout->addWidget(_bannerCarousel);
    _vboxLayout->addWidget(_content);

    _bannerCarousel->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    _bannerVboxLayout->setSpacing(0);
    _bannerVboxLayout->setContentsMargins(0, 0, 0, 0);
    _bannerVboxLayout->addStretch(1);
    _bannerVboxLayout->addWidget(_bannerTitle);
    _bannerVboxLayout->addWidget(_typewriterLabel);
    _bannerVboxLayout->addStretch(1);
    _bannerVboxLayout->addWidget(_wave);

    _bannerTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    _typewriterLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    _wave->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    QApplication::setFont(QFont("Dream Han Sans CN", 10));
    const QFont indexFont("Zen Maru Gothic");
    QPalette typewriterPal = _typewriterLabel->palette();
    typewriterPal.setColor(QPalette::ColorRole::WindowText, Qt::GlobalColor::white);
    QPalette titlePal = _bannerTitle->palette();
    titlePal.setColor(QPalette::ColorRole::WindowText, Qt::GlobalColor::white);

    _typewriterLabel->setFont(indexFont);
    _typewriterLabel->setPalette(typewriterPal);
    _typewriterLabel->start();

    _bannerTitle->setFont(indexFont);
    _bannerTitle->setPalette(titlePal);
    _bannerTitle->setAlignment(Qt::AlignmentFlag::AlignCenter);
    _bannerTitle->setText("Mizuki-Qt");

    auto *shadow = new QGraphicsDropShadowEffect(_bannerTitle);
    shadow->setOffset(2, 2);
    shadow->setBlurRadius(12);
    shadow->setColor(QColor(0, 0, 0, 128));
    _bannerTitle->setGraphicsEffect(shadow);

#ifdef Q_OS_WIN
    resize(1000, 562);
#endif

    emit _bannerCarousel->nextImageNeeded();
}

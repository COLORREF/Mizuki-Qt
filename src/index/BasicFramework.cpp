//
// Created by TX on 2026/6/12.
//

#include "BasicFramework.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "ContentWidget.h"
#include "Core/ResourceManager/ResourceManager.h"
#include "Core/SizeManager/SizeManager.h"
#include "Widgets/BannerCarouselWidget/BannerCarouselWidget.h"
#include "Widgets/TypewriterLabel/TypewriterLabel.h"
#include "Widgets/WaveWidget/WaveWidget.h"


BasicFramework::BasicFramework(QWidget *parent) :
    ScrollArea(parent),
    _scrollContent(new QWidget(this)),
    _vboxLayout(new QVBoxLayout(_scrollContent)),
    _bannerCarousel(new BannerCarouselWidget(_scrollContent)),
    _bannerVboxLayout(new QVBoxLayout(_bannerCarousel)),
    _bannerTitle(new QLabel(_bannerCarousel)),
    _typewriterLabel(new TypewriterLabel(_bannerCarousel)),
    _content(new ContentWidget(_scrollContent)),
    _wave(new WaveWidget(_bannerCarousel))
{
    setWidget(_scrollContent);
    setWidgetResizable(true);

    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &BasicFramework::setSize);
    connect(_bannerCarousel, &BannerCarouselWidget::nextImageNeeded, &ResourceManager::manager(), &ResourceManager::loadHomeImage);
    connect(&ResourceManager::manager(), &ResourceManager::homeImageReady, _bannerCarousel, &BannerCarouselWidget::setPixmap);
    connect(&ResourceManager::manager(), &ResourceManager::allFontsLoaded, this, &BasicFramework::initMainUI);
    ResourceManager::manager().loadFonts();
}

void BasicFramework::setSize() const
{
    const int vw = qRound(window()->width() / window()->devicePixelRatio());
    const double s = vw > 1280 ? qBound(0.85, vw / 2000.0, 1.0) : 1.0;
    const int titlePx = vw >= 1280 ? qRound(96.0 * s) : vw >= 768 ? qRound(64.0 * s) : vw >= 480 ? 53 : 45;
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

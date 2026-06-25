//
// Created by TX on 2026/6/12.
//

#ifndef GITHUBPAGE_BASICFRAMEWORK_H
#define GITHUBPAGE_BASICFRAMEWORK_H

#include <QLabel>
#include <QVBoxLayout>

#include "Widgets/ScrollArea/ScrollArea.h"

class ContentWidget;
class TypewriterLabel;
class WaveWidget;
class BannerCarouselWidget;


class BasicFramework : public ScrollArea
{
    Q_OBJECT
    friend class Widget;

public:
    explicit BasicFramework(QWidget *parent);

private:
    QWidget *_scrollContent;
    QVBoxLayout *_vboxLayout;
    BannerCarouselWidget *_bannerCarousel;
    QVBoxLayout *_bannerVboxLayout;
    QLabel *_bannerTitle;
    TypewriterLabel *_typewriterLabel;
    ContentWidget *_content;
    WaveWidget *_wave;

private slots:
    void setSize() const;

    void initMainUI();
};


#endif //GITHUBPAGE_BASICFRAMEWORK_H

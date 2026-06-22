//
// Created by TX on 2026/6/12.
//

#ifndef GITHUBPAGE_BASICFRAMEWORK_H
#define GITHUBPAGE_BASICFRAMEWORK_H

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

#include "Widgets/ScrollArea/ScrollArea.h"

class TypewriterLabel;
class WaveWidget;
class BannerCarouselWidget;

class ContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContentWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;
};


class BasicFramework : public ScrollArea
{
    Q_OBJECT

public:
    explicit BasicFramework(QWidget *parent);

protected:
    void resizeEvent(QResizeEvent *event) override;

    QVBoxLayout *_vboxLayout;
    BannerCarouselWidget *_bannerCarousel;
    QVBoxLayout *_bannerVboxLayout;
    QLabel *_bannerTitle;
    TypewriterLabel *_typewriterLabel;
    ContentWidget *_content;
    WaveWidget *_wave;

private:
    void setSize() const;

    QTimer _debounceTimer; // 防抖计时器

private slots:
    void initMainUI();
};


#endif //GITHUBPAGE_BASICFRAMEWORK_H

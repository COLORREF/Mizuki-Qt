//
// Created by TX on 2026/6/23.
//

#ifndef MIZUKI_QT_CONTENTWIDGET_H
#define MIZUKI_QT_CONTENTWIDGET_H

#include <QWidget>

class QVBoxLayout;
class ProfileCard;
class QHBoxLayout;

class ContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContentWidget(QWidget *parent);

    ~ContentWidget() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QHBoxLayout *_contentLayout;
    QWidget *_parcel;
    QHBoxLayout *_parcelLayout;
    QWidget *_leftBar;
    QWidget *_middleBar;
    QWidget *_rightBar;

    QVBoxLayout *_leftBarLayout;

    ProfileCard *_profileCard;

private slots:
    void setSize() const;
};


#endif //MIZUKI_QT_CONTENTWIDGET_H

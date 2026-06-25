//
// Created by TX on 2026/6/23.
//

#ifndef MIZUKI_QT_PROFILECARD_H
#define MIZUKI_QT_PROFILECARD_H

#include <QWidget>


class AvatarButton;
class QVBoxLayout;

class ProfileCard : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileCard(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout *_cardLayout;

    AvatarButton *_avatar;

private slots:
    void reSetLayout() const;
};


#endif //MIZUKI_QT_PROFILECARD_H

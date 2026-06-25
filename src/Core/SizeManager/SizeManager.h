//
// Created by TX on 2026/6/23.
//

#ifndef MIZUKI_QT_SIZEMANAGER_H
#define MIZUKI_QT_SIZEMANAGER_H
#include <QObject>


class SizeManager : public QObject
{
    Q_OBJECT

    explicit SizeManager(QObject *parent);

public:
    static SizeManager *manager();

signals:
    void sizeAdjustmentCompleted();
};


#endif //MIZUKI_QT_SIZEMANAGER_H

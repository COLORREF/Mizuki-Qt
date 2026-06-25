//
// Created by TX on 2026/6/23.
//

#include "SizeManager.h"
#include <QApplication>
#include <QWidget>

SizeManager::SizeManager(QObject *parent) :
    QObject(parent) {}

SizeManager *SizeManager::manager()
{
    static auto *m = new SizeManager(qApp);
    return m;
}
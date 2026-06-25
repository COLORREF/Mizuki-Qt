#include "widget.h"
#include "Core/SizeManager/SizeManager.h"

Widget::Widget(QWidget *parent) :
    BasicFramework(parent),
    _sizeManager(SizeManager::manager())
{
    _debounceTimer.setSingleShot(true);
    _debounceTimer.setInterval(100);
    connect(&_debounceTimer, &QTimer::timeout, this, &Widget::onSizeAdjustmentCompleted);
}

Widget::~Widget() {}

void Widget::resizeEvent(QResizeEvent *event)
{
    BasicFramework::resizeEvent(event);
    _debounceTimer.start();
}

void Widget::onSizeAdjustmentCompleted() const
{
    emit _sizeManager->sizeAdjustmentCompleted();
}

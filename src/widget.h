#ifndef WIDGET_H
#define WIDGET_H

#include "index/BasicFramework.h"

class QLabel;

class Widget : public BasicFramework
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);

    ~Widget() override;
};

#endif // WIDGET_H

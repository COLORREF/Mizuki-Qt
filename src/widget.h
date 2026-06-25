#ifndef WIDGET_H
#define WIDGET_H

#include <QTimer>
#include "index/BasicFramework.h"

class SizeManager;
class QLabel;

class Widget : public BasicFramework
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);

    ~Widget() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSizeAdjustmentCompleted() const;

private:
    QTimer _debounceTimer;

    SizeManager *_sizeManager;
};

#endif // WIDGET_H

//
// Created by TX on 2026/6/13.
//

#ifndef GITHUBPAGE_SCROLLAREASTYLE_H
#define GITHUBPAGE_SCROLLAREASTYLE_H

#include <QProxyStyle>
class QScrollArea;

class ScrollAreaStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit ScrollAreaStyle(QScrollArea *parent);
    
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};


#endif //GITHUBPAGE_SCROLLAREASTYLE_H

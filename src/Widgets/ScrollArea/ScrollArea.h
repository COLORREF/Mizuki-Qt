//
// Created by TX on 2026/6/12.
//

#ifndef GITHUBPAGE_SCROLLAREA_H
#define GITHUBPAGE_SCROLLAREA_H


#include <QScrollArea>


class ScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit ScrollArea(QWidget *parent = nullptr);
};


#endif //GITHUBPAGE_SCROLLAREA_H

#pragma once
#include "Item.h"
#include <iostream>
#include <QThread>

class Producer : public QThread//生产者类
{
    Q_OBJECT
    
    int p_id;//生产者id号，不同于进程id
    int produceSpeed = 1000;//生产者速度，后续作为sleep函数的参数，单位是毫秒
public:
    explicit Producer(int id, QObject* parent = 0);

    void run();
    
    void produceItem(ItemRepo* IR, Item* item);

    //信号量
signals:
    void updateIRInfo(const QString& text);
    void updateCurProducerCacheText(const QString& P_ID, const QString& T_ID, int number, int location);
    void end();
};

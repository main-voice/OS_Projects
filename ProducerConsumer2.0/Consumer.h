#pragma once
#include "Item.h"
#include <iostream>
#include <QThread>

class Consumer : public QThread//��������
{
    Q_OBJECT

    int c_id;
    int consumeSpeed = 1000;

public:

    explicit Consumer (int id, QObject* parent = 0);

    void run();

    Item* consumeItem(ItemRepo* IR);

//�ź���, 
signals:
    void updateIRInfo(const QString& text);
signals:
    void updateCurConsumerCacheText(const QString& C_ID, const QString& T_ID, int number, int location);
signals:
    void end();
};

#pragma once
#include "Item.h"
#include <iostream>
#include <QThread>

class Producer : public QThread//��������
{
    Q_OBJECT
    
    int p_id;//������id�ţ���ͬ�ڽ���id
    int produceSpeed = 1000;//�������ٶȣ�������Ϊsleep�����Ĳ�������λ�Ǻ���
public:
    explicit Producer(int id, QObject* parent = 0);

    void run();
    
    void produceItem(ItemRepo* IR, Item* item);

    //�ź���
signals:
    void updateIRInfo(const QString& text);
    void updateCurProducerCacheText(const QString& P_ID, const QString& T_ID, int number, int location);
    void end();
};

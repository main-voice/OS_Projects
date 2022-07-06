#include "Producer.h"
#include "GlobalHead.h"
#include "qdebug.h"

Producer::Producer(int id, QObject* parent)
    :QThread(parent)
{
    p_id = id;
}

void Producer::run()//ѭ������
{

    while (gInPosi < BUFFER_SIZE)//��ʾ����λ��С�ڻ���������������������һ�ֺ�ֹͣ
    {
        gIR->emptyL->wait();
        gIR->mtxL->wait();

        auto newItem = new Item(gIR.get()->inPos + 1);
        this->produceItem(gIR.get(), newItem); // ѭ��������Ʒ.
        
        gIR->mtxL->signal();
        gIR->fullL->signal();
    }

#if G_DEBUG
    qDebug() << this->p_id << ", this Producer thread end";
#endif // G_DEBUG

    gIR->mtxL->wait();
    QString output = "producer_ID = ";
    output.append(QString::number(p_id));
    output.append(", thread_ID = ");
    auto curThreadId = QString::number(quintptr(QThread::currentThreadId()));
    output.append(" end.\n");

    emit updateIRInfo(output);//һ�����̽��� �ź���
    emit end();//�ж��ܹ������Ƿ���� �ź���
    gIR->mtxL->signal();
}

void Producer::produceItem(ItemRepo* IR, Item* item) {

    if (gInPosi >= DataMaxSize) {
        return;
    }

    if (IR->inPos >= IR->BUFFER_SIZE) {

#if G_DEBUG
        qDebug() << "out of range in produceItem";
#endif // G_DEBUG

        return;
    }

    IR->buffer[IR->inPos] = item;

#if G_DEBUG
    for (const auto& x : IR->buffer) {
        if (!x) {
            std::cout << "-1 ";
        }
        else {
            std::cout << x->value << " ";
        }
    }
    std::cout << "\n";
#endif // G_DEBUG

#if G_DEBUG
    qDebug() << u8"current produce item : " << item->value;
    qDebug() << "place physical posi : " << IR->inPos;
#endif // G_DEBUG

    auto tempInPos = IR->inPos;
    auto tempInVal = item->value;
    IR->inPos = (IR->inPos + 1) % IR->BUFFER_SIZE;
    IR->counter++;

#if G_DEBUG
    qDebug() << "gInPosiCount: " << gInPosi;
#endif // G_DEBUG

    gInPosi++;

#if G_DEBUG
    qDebug() << "gInPosiCount: " << gInPosi;
#endif // G_DEBUG

    //�����Ϣ
    QString output = "producer_ID = ";
    output.append(QString::number(p_id));
    output.append(", thread_ID = ");
    auto curThreadId = QString::number(quintptr(QThread::currentThreadId()));
    output.append(curThreadId);
    output.append(" , produce_item_of ");
    output.append(QString::number(tempInVal));
    output.append(" in_the_buffer_position_of ");
    output.append(QString::number(tempInPos + 1));

    emit updateIRInfo(output);//���������������ź����ı���������
    emit updateCurProducerCacheText(QString::number(p_id), curThreadId, tempInVal, tempInPos);


    std::this_thread::sleep_for(std::chrono::milliseconds(produceSpeed));

    return;
}



#include "Producer.h"
#include "GlobalHead.h"
#include "qdebug.h"

Producer::Producer(int id, QObject* parent)
    :QThread(parent)
{
    p_id = id;
}

void Producer::run()//循环生产
{

    while (gInPosi < BUFFER_SIZE)//表示进入位置小于缓冲区数量，代表生产完一轮后即停止
    {
        gIR->emptyL->wait();
        gIR->mtxL->wait();

        auto newItem = new Item(gIR.get()->inPos + 1);
        this->produceItem(gIR.get(), newItem); // 循环生产产品.
        
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

    emit updateIRInfo(output);//一个进程结束 信号量
    emit end();//判断总共进程是否结束 信号量
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

    //输出信息
    QString output = "producer_ID = ";
    output.append(QString::number(p_id));
    output.append(", thread_ID = ");
    auto curThreadId = QString::number(quintptr(QThread::currentThreadId()));
    output.append(curThreadId);
    output.append(" , produce_item_of ");
    output.append(QString::number(tempInVal));
    output.append(" in_the_buffer_position_of ");
    output.append(QString::number(tempInPos + 1));

    emit updateIRInfo(output);//结束生产，发出信号让文本框对象接受
    emit updateCurProducerCacheText(QString::number(p_id), curThreadId, tempInVal, tempInPos);


    std::this_thread::sleep_for(std::chrono::milliseconds(produceSpeed));

    return;
}



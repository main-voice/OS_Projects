#include "Consumer.h"
#include "GlobalHead.h"
#include "QDebug"   // QDebug() << ...

Consumer::Consumer(int id, QObject* parent)
    : QThread(parent)
{
    c_id = id;
}

void Consumer::run()
{
    
    while (gOutPosi < BUFFER_SIZE)
    {
        gIR->fullL->wait();
        gIR->mtxL->wait();

        this->consumeItem(gIR.get()); // 循环消耗产品.
        
        gIR->mtxL->signal();
        gIR->emptyL->signal();

    }

#if G_DEBUG
    qDebug() << this->c_id << ", this Consumer thread end";
#endif // G_DEBUG

    gIR->mtxL->wait();
    QString output = "consumer_ID = ";
    output.append(QString::number(c_id));
    output.append(", thread_ID = ");
    auto curThreadId = QString::number(quintptr(QThread::currentThreadId()));
    output.append(" end.\n");

    emit updateIRInfo(output);//一个进程结束 信号量
    emit end();//判断总共进程是否结束 信号量
    gIR->mtxL->signal();

}

Item* Consumer::consumeItem(ItemRepo* IR)
{
    if (gOutPosi >= BUFFER_SIZE) {
#if G_DEBUG
        qDebug() << "gOutPosi is " << gOutPosi << " .it's out of range in consumeItem";
#endif // G_DEBUG
        return nullptr;
    }

    auto* item = IR->buffer[IR->outPos];
    if (item->value < 0) {
#if G_DEBUG
        qDebug() << "the item is invaild ,wrong posi : " << IR->outPos;
#endif // G_DEBUG
        return nullptr;
    }

#if G_DEBUG
    qDebug() << "current consume item : " << item->value;
    qDebug() << "consume physical posi : " << IR->outPos;
#endif // G_DEBUG

    if (item == nullptr) {
        qDebug() << "this item is null!, position is " << IR->outPos;
        return nullptr;
    }

    auto tempOutPos = IR->outPos;
    auto tempOutVal = item->value;

    IR->buffer[IR->outPos]->value = -1;

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


    IR->outPos = (IR->outPos + 1) % IR->BUFFER_SIZE;
    IR->counter--;

#if G_DEBUG
    qDebug() << "gOutPosiCount: " << gOutPosi;
#endif // G_DEBUG
    gOutPosi++;
#if G_DEBUG
    qDebug() << "gOutPosiCount: " << gOutPosi;
#endif // G_DEBUG

    //输出信息
    QString output = "consumer_ID = ";
    output.append(QString::number(c_id));
    output.append(", thread_ID = ");
    auto curThreadId = QString::number(quintptr(QThread::currentThreadId()));
    output.append(curThreadId);
    output.append(" , consume_item_of ");
    output.append(QString::number(tempOutVal));
    output.append(" in_the_buffer_position_of ");
    output.append(QString::number(tempOutPos));

    emit updateIRInfo(output);
    emit updateCurConsumerCacheText(QString::number(c_id), curThreadId, tempOutVal, tempOutPos);

    
    std::this_thread::sleep_for(std::chrono::milliseconds(consumeSpeed));

    return item;

}


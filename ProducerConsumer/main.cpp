#include <mutex>
#include <vector>
#include <iostream>

#ifndef CORE_CODE
#define CORE_CODE

//semaphore是整型变量，除了初始化只有wait和signal来访问，wait最初称为P（荷兰语，测试），signal最初称为V（荷兰语，增加）
class semaphore
{

private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;

public:
    semaphore(int value = 1) : count(value) { ; }
    ~semaphore() { ; }

    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        if (--count < 0) {
            cv.wait(lock);
        }
    }

    void signal() {
        std::unique_lock<std::mutex> lock(mtx);
        if (++count <= 0) {
            cv.notify_one();
        }
    }

};

class Item {
public:
    int value = -1;
    
    explicit Item(int v) {
        value = v;
    }

    Item() = default;
};

class ItemRepo {
public:
    int index;
    const int BUFFER_SIZE;

    std::vector<Item*> buffer;  //buffer of items
    size_t in = 0;
    size_t out = 0;
    size_t counter = 10; //number of current items

    semaphore* mtxL;    // 用于提供缓冲区的互斥要求
    semaphore* emptyL;  // 表示未被占用的缓冲区的数量
    semaphore* fullL;   // 表示已被占用的缓冲区的数量

public:
    ItemRepo(int index, int bufferSize)
        : index(index), BUFFER_SIZE(bufferSize) {

        buffer.resize(BUFFER_SIZE);
        emptyL = new semaphore(BUFFER_SIZE);
        fullL = new semaphore(0);
        mtxL = new semaphore(1);
    }
    virtual ~ItemRepo() {
        delete mtxL;
        delete emptyL;
        delete fullL;
    }
};


struct Producer
{
    int p_id;
    Producer(int id) {
        p_id = id;
    }
    void produceItem(ItemRepo* IR, Item* item) {
        IR->emptyL->wait();
        IR->mtxL->wait();

        IR->buffer[IR->in] = item;
        std::cout << "生产者 " << p_id << " 生产产品，位于 " << IR->in << "，进程id为 " << std::this_thread::get_id() << "\n";
        IR->in = (IR->in + 1) % IR->BUFFER_SIZE;
        IR->counter++;

        IR->mtxL->signal();
        IR->fullL->signal();
    }

};

void putTask(Producer* producer, ItemRepo* gItemRepo, int speed) {

    static int i = 0;
    while (true) {
        //int idlei = *idle;
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));
        producer->produceItem(gItemRepo, new Item(i++)); // 循环生产产品.
    }
}

struct Consumer
{
    int c_id;
    Consumer(int id) {
        c_id = id;
    }

    Item* consumeItem(ItemRepo* IR) {
        IR->fullL->wait();
        IR->mtxL->wait();

        auto* item = IR->buffer[IR->out];
        IR->buffer[IR->out] = nullptr;
        std::cout << "消费者 " << c_id << " 消耗了位于 " << IR->out << "的产品，进程id为 " << std::this_thread::get_id() << "\n";
        IR->out = (IR->out + 1) % IR->BUFFER_SIZE;
        IR->counter--;

        IR->mtxL->signal();
        IR->emptyL->signal();

        return item;

    }

};

void getTask(Consumer* consumer, ItemRepo* gItemRepo, int speed) {
    while (true) {
        //int idlei = *idle;
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));
        if (consumer->consumeItem(gItemRepo))
            delete consumer->consumeItem(gItemRepo); // 消费一个产品.
        else
            std::cout << "no item..\n";
    }
}


#endif // !CORE_CODE

ItemRepo* IR = nullptr;

int main()
{
    IR = new ItemRepo(1, 10);
    int producerNum = 6;
    std::vector<Producer*> vProducer(producerNum);

    for (size_t i = 0; i < producerNum; i++) {
        vProducer[i] = new Producer(101 + i);
    }

    int consumerNum = 6;
    std::vector<Consumer*> vConsumer(consumerNum);

    for (size_t i = 0; i < consumerNum; i++) {
        vConsumer[i] = new Consumer(201 + i);
    }
    /*Producer P1(101);
    Producer P2(102);
    Producer P3(103);
    Producer P4(104);
    Producer P5(105);
    Producer P6(106);
    std::vector<Producer*> vProducer{ &P1, &P2, &P3, &P4, &P5, &P6 };*/

    //Consumer C1(201);
    //Consumer C2(202);
    //Consumer C3(203);
    //Consumer C4(204);
    //Consumer C5(205);
    //Consumer C6(206);
    //std::vector<Consumer*> vConsumer{ &C1, &C2, &C3, &C4, &C5, &C6 };

    size_t threadNum = 6;
    //const float idle = 1;
    std::vector<std::thread> threads;

    int consumeWaitTime = 0;//ms
    int produceWaitTime = 0;//ms
    //此处1与0.9无区别，因为都是要赋值给一个int的数值，所有小于1的都被认为是0；
#if 0
    std::cout << consumeSpeed;
    std::cin.get();
#endif
    // 此处一开始存在这样的问题：如果有3个生产者，3个消费者，那么每次都是3个生产者先生产完之后，紧接着3个消费者的消费。循环往复
    // 与同学讨论后，认为核心问题还是在getTask与putTask的等待时间上。
    // 当等待时间较长时，以1s为例，会影响到进程的正常切换
    // 如果说生产者先来，由于生产者都是先等待1s，因此会首先积压3个生产者，甚至于等到3个消费者都已经积压完了，第一个1s还没等待完
    // 然后开始执行生产物品的操作
    // 生产者先wait empty，而empty为n，是足够的，可以过，之后占用mutex，第一个占用了mutex，第二个等着（即好多生产者都在等mutex）
    // 第一个执行完，先释放mutex，表示当前没有人在生产，第二个生产者就可以开始生产了（之所以不是第一个消费者开始消费，是因为消费者先等的是full，full一开始是0, 需要等待第一个生产者完全生产完，而大量生产者在等待mutex
    // 等到没有生产者等待mutex了（在代码中即表现为第二轮的生产者都在sleep_for下一秒），此时释放full，消费者过了full，但是又发现到了mutex，但是mutex还在后面的那些生产者占用着，所以消费者继续等
    // 如果是消费者先到，还是要先等full， 
    //
    for (auto& p : vConsumer) {
        threads.push_back(std::thread(getTask, p, IR, consumeWaitTime));
    }
    for (auto& p : vProducer) {
        threads.push_back(std::thread(putTask, p, IR, produceWaitTime));
    }

    //// threads[0] = std::thread(&Producer::produceItem, IR);
    //for (size_t i = 0; i < threadNum; i++)
    //{
    //    threads[i] = std::thread(getTask, vConsumer[i / 2], IR, consumeSpeed);
    //    i++;
    //    threads[i] = std::thread(putTask, vProducer[i / 2], IR, produceSpeed);
    //}

    for (size_t i = 0; i < threadNum; i++)
    {
        threads[i].join();
    }

    std::cin.get();

    delete IR;
    return 0;
}
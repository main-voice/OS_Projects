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
    float r, g, b;
    int alli, rei;

    explicit Item(int v) {
        value = v;
        r = rand() % 10 / 10.0;
        g = rand() % 10 / 10.0;
        b = rand() % 10 / 10.0;
        alli = 0;
        rei = 0;
    }

    Item() = default;
};

class ItemRepo {
public:
    const int BUFFER_SIZE;
    int index;

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
        producer->produceItem(gItemRepo, new Item(i++)); // 循环生产 kItemsToProduce 个产品.
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
        if(consumer->consumeItem(gItemRepo))
            delete consumer->consumeItem(gItemRepo); // 消费一个产品.
    }
}


#endif // !CORE_CODE

ItemRepo* IR = nullptr;

int main()
{
    IR = new ItemRepo(1, 5);
    Producer P1(101);
    Producer P2(102);
    Producer P3(103);
    Producer P4(104);
    Producer P5(105);
    Producer P6(106);
    std::vector<Producer*> vProducer{ &P1, &P2, &P3, &P4, &P5, &P6 };

    Consumer C1(201);
    Consumer C2(202);
    Consumer C3(203);
    Consumer C4(204);
    Consumer C5(205);
    Consumer C6(206);
    std::vector<Consumer*> vConsumer{ &C1, &C2, &C3, &C4, &C5, &C6 };

    size_t threadNum = 6;
    //const float idle = 1;
    std::vector<std::thread> threads;

    int consumeSpeed = 10;//ms
    int produceSpeed = 10;//ms


    for (auto& p : vProducer) {
        threads.push_back(std::thread(putTask, p, IR, produceSpeed));
    }
    for (auto& p : vConsumer) {
        threads.push_back(std::thread(getTask, p, IR, consumeSpeed));
    }

    //// threads[0] = std::thread(&Producer::produceItem, IR);
    //for (size_t i = 0; i < threadNum; i++)
    //{
    //    threads[i] = std::thread(getTask, vConsumer[i / 2], IR, consumeSpeed);
    //    i++;
    //    threads[i] = std::thread(putTask, vProducer[i / 2], IR, produceSpeed);
    //}

   /* for (size_t i = 0; i < threadNum; i++)
    {
        threads[i].join();
    }*/

    std::cin.get();

    delete IR;
    return 0;
}
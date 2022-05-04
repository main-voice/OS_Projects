#include <mutex>
#include <vector>
#include <iostream>

#ifndef CORE_CODE
#define CORE_CODE

//semaphore�����ͱ��������˳�ʼ��ֻ��wait��signal�����ʣ�wait�����ΪP����������ԣ���signal�����ΪV����������ӣ�
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

    semaphore* mtxL;    // �����ṩ�������Ļ���Ҫ��
    semaphore* emptyL;  // ��ʾδ��ռ�õĻ�����������
    semaphore* fullL;   // ��ʾ�ѱ�ռ�õĻ�����������

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
        std::cout << "������ " << p_id << " ������Ʒ��λ�� " << IR->out << "������idΪ " << std::this_thread::get_id() << "\n";
        IR->in = (IR->in + 1) % IR->BUFFER_SIZE;
        IR->counter++;

        IR->mtxL->signal();
        IR->fullL->signal();
    }


};

void putTask(Producer* producer, ItemRepo* gItemRepo, const float* idle) {

    static int i;
    while (true) {
        int idlei = *idle;
        std::this_thread::sleep_for(std::chrono::milliseconds(idlei));
        producer->produceItem(gItemRepo, new Item(i++)); // ѭ������ kItemsToProduce ����Ʒ.
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
        std::cout << "������ " << c_id << " ������λ�� " << IR->out << "�Ĳ�Ʒ������idΪ " << std::this_thread::get_id() << "\n";
        IR->out = (IR->out + 1) % IR->BUFFER_SIZE;
        IR->counter--;

        IR->mtxL->signal();
        IR->emptyL->signal();

        return item;

    }

};

void getTask(Consumer* consumer, ItemRepo* gItemRepo, const float* idle) {
    while (true) {
        int idlei = *idle;
        std::this_thread::sleep_for(std::chrono::milliseconds(idlei));
        delete consumer->consumeItem(gItemRepo); // ����һ����Ʒ.
    }
}


#endif // !CORE_CODE

ItemRepo* IR = nullptr;

int main()
{
    IR = new ItemRepo(1, 10);
    Producer P1(101);
    Producer P2(102);
    Producer P3(103);
    std::vector<Producer*> vProducer{ &P1, &P2, &P3 };

    Consumer C1(201);
    Consumer C2(202);
    Consumer C3(203);
    std::vector<Consumer*> vConsumer{ &C1, &C2, &C3 };

    size_t threadNum = 6;
    const float idle = 1;
    std::vector<std::thread> threads(6);

    // threads[0] = std::thread(&Producer::produceItem, IR);
    for (size_t i = 0; i < threadNum; i++)
    {
        threads[i] = std::thread(putTask, vProducer[i / 2], IR, &idle);

        i++;

        threads[i] = std::thread(getTask, vConsumer[i / 2], IR, &idle);
    }

    for (size_t i = 0; i < threadNum; i++)
    {
        threads[i].detach();
    }

    std::cin.get();

    delete IR;
    return 0;
}
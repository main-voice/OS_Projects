#pragma once
#include <mutex>

//semaphore�����ͱ��������˳�ʼ��ֻ��wait��signal�����ʣ�wait�����ΪP����������ԣ���signal�����ΪV����������ӣ�
class Semaphore
{

private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;

public:
    Semaphore(int value = 1)
        : count(value) {
        ;
    }

    ~Semaphore() { ; }

    void wait();

    void signal();
};
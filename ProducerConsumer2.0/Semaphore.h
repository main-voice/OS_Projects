#pragma once
#include <mutex>

//semaphore是整型变量，除了初始化只有wait和signal来访问，wait最初称为P（荷兰语，测试），signal最初称为V（荷兰语，增加）
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
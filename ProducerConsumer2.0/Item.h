#pragma once
#include <vector>
#include "Semaphore.h"

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

    size_t inPos = 0;
    size_t outPos = 0;
    
    int inVal = 0;
    int outVal = 0;

    size_t counter = 0; //number of current items

    Semaphore* mtxL;    // �����ṩ�������Ļ���Ҫ��
    Semaphore* emptyL;  // ��ʾδ��ռ�õĻ�����������
    Semaphore* fullL;   // ��ʾ�ѱ�ռ�õĻ�����������

public:
    ItemRepo(int index, int bufferSize);
    ~ItemRepo();
};


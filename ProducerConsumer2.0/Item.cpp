#include "Item.h"


ItemRepo::ItemRepo(int index, int bufferSize)
    : index(index), 
    BUFFER_SIZE(bufferSize) {

    buffer.resize(BUFFER_SIZE);

    emptyL = new Semaphore(BUFFER_SIZE);
    fullL = new Semaphore(0);
    mtxL = new Semaphore(1);
} 

ItemRepo::~ItemRepo()
{
    for (auto& x : buffer) {
        if (x) {
            delete x;
        }
    }
    delete mtxL;
    delete emptyL;
    delete fullL;
}
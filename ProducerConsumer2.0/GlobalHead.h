#pragma once

#include "Item.h"

extern const size_t BUFFER_SIZE;
extern const int DataMaxSize;//数据最大值

// global Item Repository
extern std::unique_ptr<ItemRepo> gIR; 
extern int gInPosi;
extern int gOutPosi;

//用于总进程数量，默认生产者消费者数量各自一半
extern int gThreadNum; 

extern int gVaildThreadNum;

//global debug flag
#define G_DEBUG true;


#pragma once

#include "Item.h"

extern const size_t BUFFER_SIZE;
extern const int DataMaxSize;//�������ֵ

// global Item Repository
extern std::unique_ptr<ItemRepo> gIR; 
extern int gInPosi;
extern int gOutPosi;

//�����ܽ���������Ĭ����������������������һ��
extern int gThreadNum; 

extern int gVaildThreadNum;

//global debug flag
#define G_DEBUG true;


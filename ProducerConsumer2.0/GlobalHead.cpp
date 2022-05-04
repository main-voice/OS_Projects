#include "GlobalHead.h"

const int DataMaxSize = 20;
const size_t BUFFER_SIZE = 20u;

std::unique_ptr<ItemRepo> gIR(new ItemRepo(1, BUFFER_SIZE));

int gInPosi = 0;
int gOutPosi = 0;

int gThreadNum = 6;
int gVaildThreadNum = gThreadNum;

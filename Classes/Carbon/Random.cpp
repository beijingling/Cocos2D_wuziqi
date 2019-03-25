#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void _randomize()
{
    clock_t c = clock();
    srand((int)c);              // 用程序启动的时间，强制转换成一个int
}

unsigned _random(unsigned x)
{
    return rand() % x;
}

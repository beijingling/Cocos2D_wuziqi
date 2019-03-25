#ifndef _TIMER
#define _TIMER

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

long getTime()
{
    return clock();
}

// 时间测量类
class Timer
{
public:
    Timer() {
        reset();
    }

    void start() {
        start_time = getTime();
    }

    void stop() {
        _time += (double)(getTime() - start_time) / 1000;
    }

    void reset() {
        _time = 0;
    }

    double time() {
        return _time;
    }

private:
    double start_time;
    double _time;
};

#endif

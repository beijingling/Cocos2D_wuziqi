#ifndef CARBONAIPARAMETER_H
#define CARBONAIPARAMETER_H

static int info_time_left = 1000000000;             /* left time for a game */
static int info_timeout_turn = 1000;			    /* time for one turn in milliseconds */
static int info_renju = 0;                  
static int terminateAI = 0;                         // ai 停止一个标志位，重新开始一局游戏时，值为0 ai 正常，值为非零停止AI
// static int info_max_memory = 1024 * 1024 * 350;
static int info_max_memory = 1024 * 1024 * 50;      // info 最大分配内存空间，默认350MB，这里改成50MB
static int info_exact5 = 0;

#endif // CARBONAIPARAMETER_H

/*
 *  OXTypes.h
 */

#ifndef _OXTYPES
#define _OXTYPES

 // 棋盘最大尺寸，应该是 长 × 宽， 不用这么大
const int MAX_BOARD_WIDTH = 64;
const int MAX_BOARD_HEIGHT = 32;

// 下棋的最大步数
const int MAX_CELLS = MAX_BOARD_WIDTH * MAX_BOARD_HEIGHT;

typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

// x轴，y轴，右下，左上 不同的4个方向
const int DX[4] = { 1, 0, 1, 1 };
const int DY[4] = { 0, 1, 1, -1 };

// 棋子落子坐标
struct OXPoint {
    OXPoint(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    bool operator == (const OXPoint &p) { return x == p.x && y == p.y; }
    unsigned char x, y;
};

// 棋子类型， O棋子，X棋子，EMPTY还没下棋子
enum OXPiece {
    EMPTY = 2,
    OP = 0,
    XP = 1,
    WRONG = 3
};

// 获取对手类型
#define OPPONENT(x) ((x) == OP ? XP : OP)

#endif
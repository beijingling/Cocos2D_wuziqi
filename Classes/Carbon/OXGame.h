#ifndef _OXGAME
#define _OXGAME

#include "OXTypes.h"
#include "AICarbon.h"

class ChessBoard;
class OXGame
{
public:
    OXGame();
    void    start(int bsize);                       // new game
    void    start(int x, int y);
    bool    move(int x, int y);
    void    undo();
    OXPiece cell(int x, int y) const;               // 棋盘上落子情况
    bool    isWinPoint(int x, int y) const;
    int     moveCount()        const { return _moveCount; }
    int     boardSize()        const { return _boardSize; }
    OXPoint lastMove()         const { return _move[_moveCount - 1]; }
    OXPiece player()           const { return _player; }
    bool    finished()         const { return _finished; }
    OXPiece winner()           const { return _winner; }
    OXPoint* getWinPoint()     { return _winPoint;}
    void    setBoard (ChessBoard* board) {_chessBoard = board;}
    void    setAI(AICarbon *ai)          {_AICarbon = ai;}

private:
    OXPiece   _cell[MAX_BOARD_WIDTH][MAX_BOARD_HEIGHT];             // 二维数组，棋盘
    OXPiece   _player;                                              // 当前是谁下子
    OXPoint   _move[MAX_CELLS];                                     // 数组，记录每一步落子位置
    int       _moveCount;                                           // 记录当前下了几步
    int       _boardSize;
    bool      _finished;       //when there are no space to place chess or player win.
    OXPiece   _winner;                                              // O/X 胜利
    OXPoint   _winPoint[5];                                         // 连成5个子的位置
    ChessBoard* _chessBoard;
    bool is5inRow(int x, int y);
    void reset();
    AICarbon *_AICarbon;
};

inline OXPiece OXGame::cell(int x, int y) const
{
    if (x < 0 || x >= _boardSize || y < 0 || y >= _boardSize) return WRONG;
    return _cell[x][y];
}
#endif

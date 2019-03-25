#include "OXGame.h"

#include "ChessBoard.h"
OXGame::OXGame()
{
    _moveCount = 0;
    _boardSize = 0;
    _winner = EMPTY;
    _finished = true;
}

void OXGame::start(int bsize)
{
    reset();
    _moveCount = 0;
    _boardSize = bsize;
    _winner = EMPTY;
    _finished = false;
    // clear the board
    int i, j;
    for (i = 0; i < bsize; i++) {
        for (j = 0; j < bsize; j++) {
            _cell[i][j] = EMPTY;
        }
    }
    // O先下
    _player = OP;                   // 一般执黑先下
}

void OXGame::reset(){
    for (int i = 0;i < _moveCount; i++){
        OXPoint point = _move[i];
        _chessBoard->updateBoard(point.x, point.y, EMPTY);
    }
}

void OXGame::start(int x, int y)
{
    if(x>y){
        start(y);
    } else {
        start(x);
    }
}

// 落子
bool OXGame::move(int x, int y)
{
    if (_finished) {
        if(_winner != EMPTY) {//
            _chessBoard->showWinLine(_winPoint);
        }
        return 1;
    }

    //if(_cell[x][y] != EMPTY){
    //    return 0;//check if chess been pressed
    //}

    // to update the AI steps
    _AICarbon->setWho(_player);
    _AICarbon->move(x, y);

    _cell[x][y] = _player;          // 棋盘上记录落子类型
    _move[_moveCount].x = x;        // 记录落子记录
    _move[_moveCount].y = y;
    _moveCount++;
    // to update the chess board
    _chessBoard->updateBoard(x, y, _player);
    if (is5inRow(x, y)) {
        _winner = _player;
        _finished = true;
        // to show 5 in row
        _chessBoard->showWinLine(_winPoint);
    }

    if (_moveCount == _boardSize * _boardSize) {
        _finished = true;           // 全部下满
    }

    _player = OPPONENT(_player);
    return _finished;
}

void OXGame::undo()
{
    if(_moveCount < 0)
        return;
    _AICarbon->undo();
    _player = OPPONENT(_player);     // 悔棋
    _moveCount--;
    int x, y;
    x = _move[_moveCount].x;
    y = _move[_moveCount].y;
    _cell[x][y] = EMPTY;
    _finished = false;
    _chessBoard->updateBoard(x, y, EMPTY);
    //to avoid player get more steps
    if(_moveCount %2 ==1) {
        undo();
    }
}

bool OXGame::is5inRow(int x, int y)
{
    // const int DX[4] = { 1, 0, 1, 1 };
    // const int DY[4] = { 0, 1, 1, -1 };
    //DX[0] DY[0] 表示X 轴正向检查是否在一行
    //DX[1] DY[1] 表示Y 轴正向检查是否在一行
    //DX[2] DY[2] 表示右上 轴正向检查是否在一行
    //DX[3] DY[3] 表示右下 轴正向检查是否在一行

#define bs _boardSize
    OXPiece a = _cell[x][y];
    int k, c1, c2, xx, yy;
    for (k = 0; k < 4; k++) {
        c1 = c2 = 0;
        // 正方向逐个比较
        for (xx = x + DX[k], yy = y + DY[k]; cell(xx, yy) == a; xx += DX[k], yy += DY[k]) {
            c1++;
        }
        // 相反方向逐个比较
        for (xx = x - DX[k], yy = y - DY[k]; cell(xx, yy) == a; xx -= DX[k], yy -= DY[k]) {
            c2++;
        }

        if (c1 + c2 >= 4) {  // 不包含当前，已经有4个连续的，直接判胜利，保存胜利数据
            // 将胜利的棋子保存到数组中
            xx = x - c2 * DX[k];
            yy = y - c2 * DY[k];
            for (int i = 0; i < 5; i++) {
                _winPoint[i] = OXPoint(xx, yy);
                xx += DX[k];
                yy += DY[k];
            }
            return true;
        }
    }
    return false;
#undef bs
}

bool OXGame::isWinPoint(int x, int y) const
{
    if (_winner != EMPTY) {
        OXPoint p(x, y);
        for (int i = 0; i < 5; i++) {
            if (p == _winPoint[i]) {
                return true;
            }
        }
    }

    return false;
}

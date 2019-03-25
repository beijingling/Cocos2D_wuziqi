#include "AICarbon.h"

#include "Config.h"
#include "Count5.h"
#include "Prior3.h"

#include "Random.h"
#include "Status.h"
#include "Timer.h"
#include "AICarbonParameter.h"

#include <time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MATCH_SPARE 7      //how much is time spared for the rest of game
#define TIMEOUT_PREVENT 5  //how much is minimax slower when the depth is increased

const int AICarbon::WIN_MIN = 25000;
const int AICarbon::WIN_MAX = 30000;
const int AICarbon::INF = 32000;
const int AICarbon::MAX_CAND = 256;

int   AICarbon::RANK[107];
int   AICarbon::PRIOR[256][256];
UCHAR AICarbon::STATUS4[10][10][10][10];

UCHAR AICarbon::getStatus4(UCHAR s0, UCHAR s1, UCHAR s2, UCHAR s3)
{
    int n[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    n[s0]++; n[s1]++; n[s2]++; n[s3]++;

    if (n[9] >= 1) return A;              // OOOO_
    if (n[8] >= 1) return B;              // OOO_
    if (n[7] >= 2) return B;              // XOOO_ * _OOOX
    if (n[7] >= 1 && n[6] >= 1) return C; // XOOO_ * _OO
    if (n[7] >= 1 && n[5] >= 1) return D; // XOOO_ * _OOX
    if (n[7] >= 1 && n[4] >= 1) return D; // XOOO_ * _O
    if (n[7] >= 1) return E;              // XOOO_
    if (n[6] >= 2) return F;              // OO_ * _OO
    if (n[6] >= 1 && n[5] >= 1) return G; // OO_ * _OOX
    if (n[6] >= 1 && n[4] >= 1) return G; // OO_ * _O
    if (n[6] >= 1) return H;              // OO_

    return 0;
}

int AICarbon::getRank(char cfg)
{
    int mul[5] = { 3, 7, 11, 15, 19 };
    return
        mul[4] * COUNT5[cfg][4] +
        mul[3] * COUNT5[cfg][3] +
        mul[2] * COUNT5[cfg][2] +
        mul[1] * COUNT5[cfg][1] +
        mul[0] * COUNT5[cfg][0];
}

int AICarbon::getPrior(UCHAR a, UCHAR b)
{
    return _PRIOR[a][b];
}

void AICarbon::init()
{
    nSearched = 0;

    int a, b, c, d;
    for (a = 0; a < 10; a++) {
        for (b = 0; b < 10; b++) {
            for (c = 0; c < 10; c++) {
                for (d = 0; d < 10; d++) {
                    STATUS4[a][b][c][d] = getStatus4(a, b, c, d);
                }
            }
        }
    }

    for (a = 0; a < 107; a++) {
        RANK[a] = getRank(a);
    }

    for (a = 0; a < 256; a++) {
        for (b = 0; b < 256; b++) {
            PRIOR[a][b] = getPrior(a, b);
        }
    }
}

long AICarbon::stopTime()
{
    return start_time + std::min(info_timeout_turn, info_time_left / MATCH_SPARE) - 30;
}

// 获取最优落子位置
void AICarbon::yourTurn(int &x, int &y, int depth, int time)
{
    int prevSearched;
    long t0, t1, td;
    OXMove best;

    start_time = getTime();
    int turnSearched = 0;
    table.resize(50000);
    initExact5();

    if (moveCount == 0) {// AI first return the middle position
        x = boardWidth / 2;
        y = boardHeight / 2;
        return;
    }

    if (databaseMove(x, y)) {
        return;
    }

    if (time > 0) {
        info_timeout_turn = time * 1000;
    }

    if (depth > 0) {
        if (time == 0) {
            info_timeout_turn = 1000000;
        }
        nSearched = 0;
        best = minimax(depth, true, -INF, INF);
        turnSearched = nSearched;
        x = best.x - 4;
        y = best.y - 4;
    } else {
        prevSearched = 0;
        for (depth = 2; depth <= 50; depth++) {
            t0 = getTime();

            nSearched = 0;
            best = minimax(depth, true, -INF, INF);
            turnSearched += nSearched;

            if (terminateAI && depth > 4) {
                depth = 0;      // timeout
                break;
            }
            x = best.x - 4;
            y = best.y - 4;
            table.resize(nSearched * 2);

            t1 = getTime();
            td = t1 - t0;
            if (terminateAI || t1 + TIMEOUT_PREVENT * td - stopTime() >= 0 || nSearched == prevSearched) {
                break;
            }

            WriteLog(best.value, nSearched, td == 0 ? 0 : nSearched / td, depth, true);
            prevSearched = nSearched;
        }
    }

    totalSearched += turnSearched;

    td = getTime() - start_time;
    WriteLog(best.value, nSearched, td == 0 ? 0 : turnSearched / td, depth, false);

    assert(!(x < 0 || x >= boardWidth || y < 0 || y >= boardHeight));
}

int AICarbon::evaluate()
{
    int a, i, k, p[2] = { 0, 0 };
    for (i = 0; i < remCount; i++)
    {
        OXCell* c = remCell[i];
        a = c->piece;
        for (k = 0; k < 4; k++)
        {
            p[a] += RANK[CONFIG[c->pattern[k][a]][c->pattern[k][1 - a]]];
        }
    }
    return p[who] - p[opp];
}

void AICarbon::generateCand(OXCand *cnd, int &nCnd)
{
    int i, x, y;

    cnd[0].x = -1;
    nCnd = 0;

    if (table.present() && table.depth() >= 0 && table.best().x != 0)
    {
        cnd[0].x = table.best().x;
        cnd[0].y = table.best().y;
        cnd[0].value = 10000;
        assert(cnd[0].x != 0 && cnd[0].y != 0);
        assert(cell[cnd[0].x][cnd[0].y].piece == EMPTY);
        nCnd = 1;
    }

    FOR_EVERY_CAND(x, y)
        if (x != cnd[0].x || y != cnd[0].y)
        {
            cnd[nCnd].x = x;
            cnd[nCnd].y = y;
            cnd[nCnd].value = cell[x][y].prior();
            if (cnd[nCnd].value > 1) nCnd++;
            assert(nCnd < MAX_CAND);
        }

#define ONE_CAND(plr, st)\
    {\
      i = 0; while (cell[cnd[i].x][cnd[i].y].status4[plr] != st) i++;\
      cnd[0] = cnd[i];\
      nCnd = 1;\
      return;\
    }

    if (nSt[who][A] > 0) ONE_CAND(who, A);
    if (nSt[opp][A] > 0) ONE_CAND(opp, A);
    if (nSt[who][B] > 0) ONE_CAND(who, B);

    if (nSt[opp][B] > 0)
    {
        nCnd = 0;
        FOR_EVERY_CAND(x, y)
            if (cell[x][y].status4[who] >= E && cell[x][y].status4[who] != FORBID || cell[x][y].status4[opp] >= E && cell[x][y].status4[opp] != FORBID)
            {
                cnd[nCnd].x = x;
                cnd[nCnd].y = y;
                cnd[nCnd].value = cell[x][y].prior();
                if (cnd[nCnd].value > 0) nCnd++;
            }
        return;
    }
}

int AICarbon::quickWinSearch()
{
    int x, y, q;
    if (nSt[who][A] >= 1) return 1;
    if (nSt[opp][A] >= 2) return -2;
    if (nSt[opp][A] == 1)
    {
        FOR_EVERY_CAND(x, y)
            if (cell[x][y].status4[opp] == A)
            {
                if (info_renju && who == firstPlayer && cell[x][y].status4[who] == FORBID) return -2; //cannot defend

                _move(x, y);
                q = -quickWinSearch();
                undo();
                if (q < 0) q--; else if (q > 0) q++;
                return q;
            }
    }
    if (nSt[who][B] >= 1) return 3; 
    if (nSt[who][C] >= 1)
    {
        if (nSt[opp][B] == 0 && nSt[opp][C] == 0 && nSt[opp][D] == 0 && nSt[opp][E] == 0)
            return 5;

        FOR_EVERY_CAND(x, y)
            if (cell[x][y].status4[who] == C)
            {
                _move(x, y);
                q = -quickWinSearch();
                undo();
                if (q > 0) return q + 1;
            }
    }
    if (nSt[who][F] >= 1)
    {
        if (nSt[opp][B] == 0 && nSt[opp][C] == 0 && nSt[opp][D] == 0 && nSt[opp][E] == 0)
            return 5;
    }
    return 0;
}

inline int candComp(const void *a, const void *b)
{
    return ((OXCand*)b)->value - ((OXCand*)a)->value;
}

inline short AICarbon::OXCell::prior()
{
    return  PRIOR[pattern[0][0]][pattern[0][1]] +
        PRIOR[pattern[1][0]][pattern[1][1]] +
        PRIOR[pattern[2][0]][pattern[2][1]] +
        PRIOR[pattern[3][0]][pattern[3][1]] +
        PRIOR[pattern[0][1]][pattern[0][0]] +
        PRIOR[pattern[1][1]][pattern[1][0]] +
        PRIOR[pattern[2][1]][pattern[2][0]] +
        PRIOR[pattern[3][1]][pattern[3][0]] +
        (adj1 != 0);
}

OXMove AICarbon::minimax(int h, bool root, int alpha, int beta)
{
    if (alpha > beta + 1) return OXMove(0, 0, beta + 1);
    OXMove best(0, 0, alpha - 1);

    int  i, x, y, value;

    static int cnt;
    if (--cnt < 0) {
        cnt = 1000;
        if (getTime() - stopTime() > 0) {
            terminateAI = 2;
        }
    }

    int q = quickWinSearch();
    if (q != 0)
    {
        if (!root)
            return OXMove(0, 0, (q > 0 ? +WIN_MAX : -WIN_MAX) - q);
        if (q == 1)
        {
            FOR_EVERY_CAND(x, y)
                if (cell[x][y].status4[who] == A)
                    return OXMove(x, y, WIN_MAX - 1);
        }
    }

    if (h == 0)
    {
        return OXMove(0, 0, evaluate());
    }

    h--;

    OXCand  cnd[MAX_CAND];
    int     nCnd;

    generateCand(cnd, nCnd);
    if (nCnd > 1) {
        qsort(cnd, nCnd, sizeof(OXCand), candComp);
    }
    else if (nCnd == 1)
    {
        if (root) {
            return OXMove(cnd[0].x, cnd[0].y, 0);
        }
    }
    else
    {
    noCand:
        FOR_EVERY_CAND(x, y) if (nCnd < MAX_CAND) cnd[nCnd++] = OXCand(x, y, 0);
        if (nCnd == 0)
            best.value = 0; // board is full
    }

    int nForbid = 0;

    for (i = 0; i < nCnd; i++)
    {
        if (info_renju && who == firstPlayer && cell[cnd[i].x][cnd[i].y].status4[who] == FORBID) {
            nForbid++;
            continue;
        }

        table.move(cnd[i].x, cnd[i].y, who);

        assert(best.value <= beta);

        if (table.present() && (table.depth() >= h && ((table.depth() ^ h) & 1) == 0 || abs(table.value()) >= WIN_MIN))
        {
            nSearched++;
            assert(table.moves() == moveCount + 1);
            value = table.value();

            table.undo(cnd[i].x, cnd[i].y, who);
        }
        else
        {
            short  vA, vB;
            OXMove m;

            _move(cnd[i].x, cnd[i].y, false);

            vA = -beta; 
            vB = -(best.value + 1);

            if (vB >= +WIN_MIN) vB++;
            if (vA <= -WIN_MIN) vA--;

            m = minimax(h, false, vA, vB);
            value = -m.value;

            if (value >= +WIN_MIN) value--;
            if (value <= -WIN_MIN) value++;

            if (-vB <= value && value <= -vA && !terminateAI)
                table.update(value, h, moveCount, m);

            undo();
        }

        if (value > best.value)
        {
            best = OXMove(cnd[i].x, cnd[i].y, value);
            if (value > beta) {
                return OXMove(best.x, best.y, beta + 1);
            }
        }

        if (terminateAI)
            break;
    }

    if (info_renju && nForbid == nCnd) {
        nCnd = 0;
        goto noCand;
    }
    return best;
}

void AICarbon::start(int size)
{
    start(size, size);
}

// 初始化AI
void AICarbon::start(int width, int height)
{
    int x, y, xx, yy, k;
    UCHAR p;

    _randomize();

    boardWidth = width;
    boardHeight = height;

    for (y = 0; y < height + 8; y++) {
        for (x = 0; x < width + 8; x++) {
            (x < 4 || y < 4 || x >= width + 4 || y >= height + 4) ?
                cell[x][y].piece = WRONG :
                cell[x][y].piece = EMPTY;

            for (k = 0; k < 4; k++) {
                cell[x][y].pattern[k][0] = cell[x][y].pattern[k][1] = 0;
            }
        }
    }

    for (y = 4; y < height + 4; y++) {
        for (x = 4; x < width + 4; x++) {
            for (k = 0; k < 4; k++) {
                xx = x - DX[k];
                yy = y - DY[k];
                for (p = 8; p != 0; p >>= 1) {
                    if (cell[xx][yy].piece == WRONG) {
                        cell[x][y].pattern[k][0] |= p;
                    }
                    if (cell[xx][yy].piece == WRONG) {
                        cell[x][y].pattern[k][1] |= p;
                    }
                    xx -= DX[k];
                    yy -= DY[k];
                }
                xx = x + DX[k];
                yy = y + DY[k];
                for (p = 16; p != 0; p <<= 1) {
                    if (cell[xx][yy].piece == WRONG) cell[x][y].pattern[k][0] |= p;
                    if (cell[xx][yy].piece == WRONG) cell[x][y].pattern[k][1] |= p;
                    xx += DX[k]; yy += DY[k];
                }
            }
        }
    }

    for (y = 4; y < height + 4; y++) {
        for (x = 4; x < width + 4; x++) {
            cell[x][y].update1(0);
            cell[x][y].update1(1);
            cell[x][y].update1(2);
            cell[x][y].update1(3);
            cell[x][y].update4();
            cell[x][y].adj1 = cell[x][y].adj2 = 0;
        }
    }

    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 10; j++) {
            nSt[i][j] = 0;
        }
    }

    totalSearched = 0;
    who = OP;
    opp = XP;
    moveCount = remCount = 0;

    upperLeftCand = OXPoint(99, 99);
    lowerRightCand = OXPoint(0, 0);

    table.clear();
}

// xp, yp in <0, boardSize)
// AI 记录棋盘上落子情况
void AICarbon::move(int xp, int yp)
{
    table.resize(1);
    initExact5();
    _move(xp + 4, yp + 4);
}

void AICarbon::setWho(OXPiece _who)
{
    who = _who;
    opp = OPPONENT(who);
    if (moveCount == 0) {
        firstPlayer = _who;
    }
}

void AICarbon::initExact5()
{
    int i, j, k;
    unsigned a, b, v;
    char y;
    static int last_exact5 = 0;

    if (info_exact5 == last_exact5) return;
    last_exact5 = info_exact5;

    y = info_exact5 ? 0 : 9;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            v = 0xf8 | j;
            a = ((v >> i) | (v << (8 - i))) & 0xff;
            for (k = 0; k < 8; k++) {
                b = ((k >> i) | (k << (8 - i))) & 0xff;
                assert(STATUS1[a][b] == 0 || STATUS1[a][b] == 9);
                STATUS1[a][b] = y;
            }
        }
    }
}

inline void AICarbon::OXCell::update1(int k)
{
    status1[k][0] = STATUS1[pattern[k][0]][pattern[k][1]];
    status1[k][1] = STATUS1[pattern[k][1]][pattern[k][0]];
}

void AICarbon::OXCell::update4()
{
    status4[0] = STATUS4[status1[0][0]][status1[1][0]][status1[2][0]][status1[3][0]];
    status4[1] = STATUS4[status1[0][1]][status1[1][1]][status1[2][1]][status1[3][1]];
}

// xp, yp in <4, boardSize + 4)
void AICarbon::_move(int xp, int yp, bool updateHash)
{
    nSearched++;

    int x, y, k;
    UCHAR p;

    assert(check());
    nSt[0][cell[xp][yp].status4[0]]--;
    nSt[1][cell[xp][yp].status4[1]]--;

    cell[xp][yp].piece = who;
    remCell[remCount] = &cell[xp][yp];
    remMove[moveCount] = OXPoint(xp, yp);
    remULCand[remCount] = upperLeftCand;
    remLRCand[remCount] = lowerRightCand;
    moveCount++;
    remCount++;

    if (xp - 2 < upperLeftCand.x) {
        upperLeftCand.x = std::max(xp - 2, 4);
    }

    if (yp - 2 < upperLeftCand.y) {
        upperLeftCand.y = std::max(yp - 2, 4);
    }

    if (xp + 2 > lowerRightCand.x) {
        lowerRightCand.x = std::min(xp + 2, boardWidth + 3);
    }

    if (yp + 2 > lowerRightCand.y) {
        lowerRightCand.y = std::min(yp + 2, boardHeight + 3);
    }
      
    for (k = 0; k < 4; k++) {
        x = xp; y = yp;
        for (p = 16; p != 0; p <<= 1) {
            x -= DX[k]; y -= DY[k];
            cell[x][y].pattern[k][who] |= p;
            if (cell[x][y].piece == EMPTY /* && (cell[x][y].adj1 || cell[x][y].adj2)*/) {
                cell[x][y].update1(k);
                nSt[0][cell[x][y].status4[0]]--;
                nSt[1][cell[x][y].status4[1]]--;
                cell[x][y].update4();
                if (info_renju) {
                    checkForbid(x, y);
                }
                nSt[0][cell[x][y].status4[0]]++;
                nSt[1][cell[x][y].status4[1]]++;
            }
        }
        x = xp; y = yp;
        for (p = 8; p != 0; p >>= 1) {
            x += DX[k];
            y += DY[k];
            cell[x][y].pattern[k][who] |= p;
            if (cell[x][y].piece == EMPTY /* && (cell[x][y].adj1 || cell[x][y].adj2) */) {
                cell[x][y].update1(k);
                nSt[0][cell[x][y].status4[0]]--;
                nSt[1][cell[x][y].status4[1]]--;
                cell[x][y].update4();
                if (info_renju) {
                    checkForbid(x, y);
                }
                nSt[0][cell[x][y].status4[0]]++;
                nSt[1][cell[x][y].status4[1]]++;
            }
        }
    }

    cell[xp - 1][yp - 1].adj1++; cell[xp][yp - 1].adj1++; cell[xp + 1][yp - 1].adj1++;
    cell[xp - 1][yp].adj1++;                              cell[xp + 1][yp].adj1++;
    cell[xp - 1][yp + 1].adj1++; cell[xp][yp + 1].adj1++; cell[xp + 1][yp + 1].adj1++;
    cell[xp - 2][yp - 2].adj2++; cell[xp][yp - 2].adj2++; cell[xp + 2][yp - 2].adj2++;
    cell[xp - 2][yp].adj2++;                              cell[xp + 2][yp].adj2++;
    cell[xp - 2][yp + 2].adj2++; cell[xp][yp + 2].adj2++; cell[xp + 2][yp + 2].adj2++;

    if (updateHash) table.move(xp, yp, who);

    who = OPPONENT(who);
    opp = OPPONENT(opp);

    assert(check());
}

void AICarbon::undo()
{
    int x, y, k;
    UCHAR p;
    int xp, yp;

    assert(check());

    moveCount--;
    remCount--;
    xp = remMove[moveCount].x;
    yp = remMove[moveCount].y;
    upperLeftCand = remULCand[remCount];
    lowerRightCand = remLRCand[remCount];

    OXCell* c = remCell[remCount];
    c->update1(0);
    c->update1(1);
    c->update1(2);
    c->update1(3);
    c->update4();
    if (info_renju) checkForbid(xp, yp);

    nSt[0][c->status4[0]]++;
    nSt[1][c->status4[1]]++;

    assert(c->piece == OP || c->piece == XP);
    c->piece = EMPTY;

    who = OPPONENT(who);
    opp = OPPONENT(opp);

    table.undo(xp, yp, who);

    for (k = 0; k < 4; k++) {
        x = xp; y = yp;
        for (p = 16; p != 0; p <<= 1) {
            x -= DX[k]; y -= DY[k];
            cell[x][y].pattern[k][who] ^= p;
            if (cell[x][y].piece == EMPTY /* && (cell[x][y].adj1 || cell[x][y].adj2) */) {
                cell[x][y].update1(k);
                nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
                cell[x][y].update4();
                if (info_renju) checkForbid(x, y);
                nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
        x = xp; y = yp;
        for (p = 8; p != 0; p >>= 1)
        {
            x += DX[k]; y += DY[k];
            cell[x][y].pattern[k][who] ^= p;
            if (cell[x][y].piece == EMPTY)// && (cell[x][y].adj1 || cell[x][y].adj2))
            {
                cell[x][y].update1(k);
                nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
                cell[x][y].update4();
                if (info_renju) checkForbid(x, y);
                nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
    }

    cell[xp - 1][yp - 1].adj1--; cell[xp][yp - 1].adj1--; cell[xp + 1][yp - 1].adj1--;
    cell[xp - 1][yp].adj1--;                              cell[xp + 1][yp].adj1--;
    cell[xp - 1][yp + 1].adj1--; cell[xp][yp + 1].adj1--; cell[xp + 1][yp + 1].adj1--;
    cell[xp - 2][yp - 2].adj2--; cell[xp][yp - 2].adj2--; cell[xp + 2][yp - 2].adj2--;
    cell[xp - 2][yp].adj2--;                              cell[xp + 2][yp].adj2--;
    cell[xp - 2][yp + 2].adj2--; cell[xp][yp + 2].adj2--; cell[xp + 2][yp + 2].adj2--;

    assert(check());
}

int AICarbon::undo(int x, int y)
{
    if (moveCount > 0 && remMove[moveCount - 1].x == x + 4 && remMove[moveCount - 1].y == y + 4) {
        undo();
        return 0;
    }
    return 1;
}

void AICarbon::block(int x, int y)
{
    int xp, yp, k;
    UCHAR p;

    xp = x + 4; yp = y + 4;

    assert(check());
    nSt[0][cell[xp][yp].status4[0]]--;
    nSt[1][cell[xp][yp].status4[1]]--;

    cell[xp][yp].piece = WRONG;
    remMove[moveCount] = OXPoint(xp, yp);
    moveCount++;
     
    for (k = 0; k < 4; k++) {
        x = xp; y = yp;
        for (p = 16; p != 0; p <<= 1) {
            x -= DX[k]; y -= DY[k];
            cell[x][y].pattern[k][0] |= p;
            cell[x][y].pattern[k][1] |= p;
            if (cell[x][y].piece == EMPTY) {
                cell[x][y].update1(k);
                nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
                cell[x][y].update4();
                if (info_renju) checkForbid(x, y);
                nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
        x = xp; y = yp;
        for (p = 8; p != 0; p >>= 1) {
            x += DX[k]; y += DY[k];
            cell[x][y].pattern[k][0] |= p;
            cell[x][y].pattern[k][1] |= p;
            if (cell[x][y].piece == EMPTY) {
                cell[x][y].update1(k);
                nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
                cell[x][y].update4();
                if (info_renju) checkForbid(x, y);
                nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
    }

    who = OPPONENT(who);
    opp = OPPONENT(opp);

    assert(check());
}

void AICarbon::checkForbid(int x, int y)
{
    int k, n, s, x1, y1, n3, n4, n6;

    OXCell *c = &cell[x][y];
    if (c->status4[firstPlayer] < F) return;

    if (c->status4[firstPlayer] == A) {
        n6 = 0;
        for (k = 0; k < 4; k++) {
            if (c->status1[k][firstPlayer] >= 9) {
                n = -1;
                x1 = x; y1 = y;
                do {
                    x1 -= DX[k]; y1 -= DY[k];
                    n++;
                } while (cell[x1][y1].piece == firstPlayer);
                x1 = x;
                y1 = y;
                do {
                    x1 += DX[k]; y1 += DY[k];
                    n++;
                } while (cell[x1][y1].piece == firstPlayer);
                if (n >= 5) {
                    if (n == 5) {
                        return; // five in a row
                    }
                    n6++;
                }
            }
        }
        if (n6 > 0) {
            // overline
            c->status4[firstPlayer] = FORBID;
        }
        return;
    }

    n3 = n4 = 0;
    for (k = 0; k < 4; k++) {
        s = c->status1[k][firstPlayer];
        if (s >= 7) {
            n4++;
        }
        else if (s >= 6) {
            n3++;
        }
    }
    if (n4 > 1 || n3 > 1) {
        // double-four or double-three
        c->status4[firstPlayer] = FORBID;
    }
}

bool AICarbon::check()
{
    int n[2][10];
    int i, j, x, y;
    for (i = 0; i <= 1; i++) for (j = 0; j < 10; j++) n[i][j] = 0;
    FOR_EVERY_CAND(x, y)
    {
        n[0][cell[x][y].status4[0]]++;
        n[1][cell[x][y].status4[1]]++;
    }

    for (i = 0; i < 2; i++) {
        for (j = 1; j < 10; j++) {
            if (n[i][j] != nSt[i][j]) {
                return false;
            }
        }
    }
    return true;
}

static signed char data[] = {
    15, 1, 4, 0, 0, 0, 1, 1, 2, 3, 2, 2, 4, 2, 5, 3, 3, 4, 3, 3, 5, 5, 4, 3, 3, 6, 4, 5, 6, 5, 5, 4, 3, 1,
    11, 1, 1, 1, 0, 0, 2, 2, 3, 2, 3, 3, 1, 3, 2, 4, 1, 5, 3, 3, 3, 2, 5, 1, 4, 3,
    11, 1, 1, 0, 0, 0, 1, 1, 3, 2, 2, 2, 2, 3, 3, 3, 2, 4, 2, 5, 1, 5, 1, 4, 0, 3,
    9, 1, 0, 0, 0, 3, 0, 1, 1, 2, 0, 2, 2, 3, 3, 3, 3, 4, 2, 4, 1, 1,
    9, 1, 1, 1, 0, 0, 2, 2, 3, 2, 3, 3, 2, 3, 1, 4, 1, 3, 1, 5, 2, 4,
    9, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 2, 3, 1, 2, 0, 3, 1, 4,
    9, 1, 0, 0, 2, 0, 1, 0, 1, 1, 0, 1, 2, 2, 3, 2, 3, 3, 2, 3, 0, 2,
    9, 1, 1, 1, 0, 0, 2, 2, 0, 2, 2, 1, 3, 1, 1, 2, 1, 3, 0, 3, 3, 0,
    9, 1, 0, 0, 2, 0, 1, 1, 1, 2, 2, 1, 1, 4, 3, 2, 4, 3, 2, 3, 0, 1,
    8, 1, 0, 2, 1, 1, 1, 2, 2, 0, 2, 2, 3, 0, 3, 1, 3, 2, 4, 0,
    8, 1, 1, 1, 3, 0, 0, 2, 3, 1, 1, 3, 2, 2, 2, 3, 1, 4, 3, 3,
    7, 1, 0, 0, 1, 1, 0, 1, 2, 2, 3, 2, 3, 3, 2, 3, 0, 2,
    7, 1, 3, 2, 2, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1, 2,
    7, 1, 0, 0, 0, 1, 1, 0, 0, 3, 2, 1, 3, 2, 1, 2, 1, -1,
    7, 1, 1, 0, 0, 1, 0, 2, 2, 2, 2, 1, 3, 2, 5, 2, 3, 0,
    7, 1, 0, 0, 0, 1, 2, 0, 0, 3, 2, 1, 1, 3, 1, 2, 2, 3,
    7, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 2, 1, 2, 1, 3, 2, 3,
    7, 1, 0, 1, 1, 1, 2, 0, 0, 2, 1, 2, 3, 4, 2, 3, 2, 1,
    6, 1, 1, 0, 0, 0, 0, 2, 1, 1, 3, 3, 2, 2, 1, -1,
    6, 1, 1, 0, 0, 0, 1, 1, 2, 1, 2, 2, 1, 2, 0, -1,
    6, 1, 0, 0, 1, 0, 2, 1, 3, 1, 3, 2, 2, 2, 1, -1,
    6, 1, 3, 0, 0, 0, 1, 1, 2, 1, 2, 2, 1, 2, -1, 0,
    6, 1, 0, 0, 1, 1, 3, 1, 2, 2, 4, 3, 3, 3, 2, 4,
    6, 1, 0, 2, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 3, 0,
    6, 1, 2, 1, 0, 0, 2, 2, 0, 1, 2, 3, 1, 2, 2, 0,
    6, 1, 2, 1, 0, 0, 1, 3, 0, 1, 2, 3, 1, 2, 2, 0,
    5, 2, 2, 2, 0, 0, 1, 0, 1, 1, 0, 1, 2, 1, 1, 2,
    5, 1, 0, 0, 2, 1, 0, 1, 2, 3, 1, 2, 1, 3,
    5, 1, 1, 0, 0, 0, 1, 1, 1, 2, 2, 1, 1, -1,
    5, 1, 1, 1, 1, 0, 0, 2, 1, 3, 1, 2, 2, 2,
    5, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 3, 2, 2,
    5, 2, 0, 0, 1, 1, 1, 2, 0, 1, 2, 1, 2, 0, 3, 0,
    5, 1, 0, 2, 1, 0, 2, 0, 2, 1, 1, 1, -1, 3,
    5, 1, 1, 0, 1, 1, 0, 1, 2, 1, 3, 2, 2, -1,
    5, 1, 1, 1, 0, 0, 2, 1, 0, 2, 3, 1, 0, 1,
    5, 2, 1, 1, 0, 0, 2, 1, 1, 3, 1, 2, 0, 1, 3, 1,
    5, 2, 1, 0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 2, -1, 2,
    5, 1, 1, 0, 1, 2, 0, 1, 3, 2, 2, 1, 2, -1,
    5, 2, 0, 1, 1, 2, 2, 1, 3, 0, 0, 3, 1, 1, 0, 2,
    5, 1, 2, 0, 0, 2, 1, 1, 1, 2, 2, 2, 0, 0,
    5, 1, 0, 0, 1, 1, 1, 0, 1, 2, 0, 2, 0, 1,
    4, 1, 1, 0, 0, 1, 1, 3, 1, 2, -1, 0,
    4, 1, 1, 0, 0, 1, 1, 1, 1, 2, -1, 0,
    4, 1, 1, 0, 1, 1, 0, 2, 2, 2, 0, 0,
    4, 1, 2, 0, 0, 0, 0, 2, 1, 1, 2, 2,
    4, 1, 0, 0, 1, 1, 3, 1, 2, 2, 3, 3,
    4, 2, 2, 0, 0, 0, 2, 2, 1, 1, 0, -1, 1, -1,
    4, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2, 2,
    4, 1, 1, 0, 0, 1, 2, 0, 1, 2, -1, 0,
    4, 1, 1, 0, 0, 0, 0, 2, 1, 1, -1, -1,
    4, 1, 1, 0, 0, 1, 2, 2, 1, 2, -1, 0,
    4, 1, 0, 0, 1, 1, 2, 1, 2, 2, 3, 3,
    3, 1, 0, 1, 0, 0, 1, 0, 1, 1,
    3, 1, 0, 0, 2, 0, 1, 1, 2, 2,
    3, 4, 1, 0, 0, 1, 1, 2, 0, 2, 0, 0, 2, 0, 2, 2,
    3, 1, 1, 2, 0, 0, 1, 1, 1, 3,
    3, 1, 1, 0, 0, 0, 1, 1, 1, 2,
    3, 1, 0, 0, 0, 2, 0, 1, 0, -1,
    3, 1, 0, 0, 2, 1, 1, 1, 2, 2,
    3, 1, 0, 0, 0, 1, 1, 2, 1, 1,
    3, 2, 0, 0, 2, 2, 1, 1, 2, 1, 1, 2,
    3, 1, 0, 1, 3, 0, 2, 0, 1, 1,
    3, 1, 0, 0, 2, 1, 2, 2, 1, 1,
    3, 4, 0, 0, 1, 1, 2, 2, 1, -1, -1, 1, 3, 1, 1, 3,
    3, 1, 0, 0, 0, 2, 1, 2, 1, 0,
    3, 2, 1, 0, 0, 3, 1, 2, 2, 3, 1, 1,
    3, 2, 0, 0, 0, 3, 0, 2, 1, 3, -1, 3,
    3, 1, 0, 0, 0, 3, 1, 2, 1, 1,
    3, 1, 1, 0, 0, 2, 1, 2, 1, 1,
    3, 1, 0, 0, 2, 1, 1, 2, 0, 2,
    3, 1, 0, 0, 2, 2, 1, 2, 0, 1,
    3, 1, 0, 0, 3, 3, 2, 2, 1, 1,
    3, 1, 0, 0, 3, 2, 2, 2, 1, 1,
    3, 1, 0, 0, 3, 1, 2, 2, 3, 3,
    3, 6, 0, 0, 1, 0, 2, 0, 1, 1, 1, -1, 0, 2, 0, -2, 2, 2, 2, -2,
    3, 3, 0, 0, 3, 2, 2, 1, 1, 0, 1, 1, 0, 1,
    2, 3, 0, 0, 1, 1, 0, 2, 2, 0, 1, 2,
    2, 8, 0, 0, 1, 0, -1, -1, 0, -1, 1, -1, 2, -1, -1, 1, 0, 1, 1, 1, 2, 1,
    2, 2, 0, 0, 2, 2, 1, 3, 3, 1,
    1, 8, 0, 0, -1, 0, 1, 0, 0, 1, 0, -1, 1, 1, -1, 1, 1, -1, -1, -1,
    0, 0
};

bool AICarbon::databaseMove(int &x0, int &y0)
{
    signed char *s, *sn;
    int i, x, y, x1, y1, flip, len1, len2, left, top, right, bottom;

    //board rectangle
    left = upperLeftCand.x + 2;
    top = upperLeftCand.y + 2;
    right = lowerRightCand.x - 2;
    bottom = lowerRightCand.y - 2;
    //find current board in the database
    for (s = data;; s = sn) {
        len1 = *s++;
        len2 = *s++;
        sn = s + 2 * (len1 + len2);
        if (len1 != moveCount) {
            if (len1 < moveCount) return false; //data must be sorted by moveCount descending
            continue;
        }
        //try all symmetries
        for (flip = 0; flip < 8; flip++) {
            for (i = 0;; i++) {
                x1 = s[2 * i];
                y1 = s[2 * i + 1];
                if (i == len1) {
                    s += 2 * (len1 + _random(len2));
                    x1 = *s++;
                    y1 = *s;
                }
                switch (flip) {
                case 0: x = left + x1; y = top + y1; break;
                case 1: x = right - x1; y = top + y1; break;
                case 2: x = left + x1; y = bottom - y1; break;
                case 3: x = right - x1; y = bottom - y1; break;
                case 4: x = left + y1; y = top + x1; break;
                case 5: x = right - y1; y = top + x1; break;
                case 6: x = left + y1; y = bottom - x1; break;
                default: x = right - y1; y = bottom - x1; break;
                }
                const int B = 4; //distance from border
                if (x - 4 < B || x - 4 >= boardWidth - B || y - 4 < B || y - 4 >= boardHeight - B) break;
                if (i == len1) {
                    x0 = x - 4;
                    y0 = y - 4;
                    return true;
                }
                //compare current board and database
                if (cell[x][y].piece != ((i & 1) ? XP : OP)) break;
            }
        }
    }
}
void AICarbon::print() {
    for(int i = 0; i<boardHeight; i++){
        for(int j = 0; j <boardWidth; j++) {
            if(cell[i][j].piece == OP){
                std::cout<<"*";
            } else if(cell[i][j].piece == XP) {
                std::cout<<"#";
            } else {
                std::cout<<"/";
            }
            std::cout<<"\n";
        }
    }
}
#ifndef _AICARBON
#define _AICARBON

#include "OXTypes.h"
#include "OXPlayer.h"

#include "AICarbonHash.h"

enum LNUM { A = 8, B = 7, C = 6, D = 5, E = 4, F = 3, G = 2, H = 1, FORBID = 9 };

#define FOR_EVERY_CAND(x, y)                                                \
  for (y = upperLeftCand.y; y <= lowerRightCand.y; y++)                     \
    for (x = upperLeftCand.x; x <= lowerRightCand.x; x++)                   \
      if (cell[x][y].piece == EMPTY && (cell[x][y].adj1 || cell[x][y].adj2))

struct OXMove
{
    OXMove() {}
    OXMove(int _x, int _y, int v) : x(_x), y(_y), value(v) {}
    operator OXPoint() { return OXPoint(x, y); }
    short x, y;
    int   value;
};

typedef OXMove OXCand;

class AICarbon : public OXPlayer
{
public:

    void start(int size);
    void start(int width, int height);
    void move(int x, int y);
    void yourTurn(int &x, int &y, int depth = 0, int time = 0);
    void undo();
    int undo(int x, int y);
    void block(int x, int y);
    void setWho(OXPiece _who);
    const char* name() const { return "AICarbon"; }
    AICarbon() { init(); }
    void print();
#ifdef DEBUG_EVAL
    void eval(int x, int y);
#endif

private:
    static const int WIN_MIN;
    static const int WIN_MAX;
    static const int INF;
    static const int MAX_CAND;

    struct OXCell
    {
        void    update4();
        void    update1(int k);

        OXPiece piece;
        UCHAR   pattern[4][2];
        UCHAR   status1[4][2];
        UCHAR   status4[2];

        char    adj1, adj2;

        short   prior();
    };

    friend  struct OXCell;

    void    _move(int x, int y, bool updateHash = true);

    void    init();
    int     getRank(char cfg);
    int     getPrior(UCHAR a, UCHAR b);
    UCHAR   getStatus4(UCHAR s0, UCHAR s1, UCHAR s2, UCHAR s3);
    void    initExact5();
    bool    databaseMove(int &x0, int &y0);

    int     evaluate();
    void    checkForbid(int x, int y);

    OXMove  minimax(int h, bool root, int alpha, int beta);

    int     quickWinSearch();

    void    generateCand(OXCand *cnd, int &nCnd);

    OXCell  cell[MAX_BOARD_WIDTH + 8][MAX_BOARD_HEIGHT + 8];
    int     boardWidth, boardHeight;
    int     moveCount;
    int     remCount;
    OXPiece who, opp;
    OXPiece firstPlayer;
    int     nSt[2][10];

    // history stack
    OXPoint remMove[MAX_CELLS];
    OXCell* remCell[MAX_CELLS];
    OXPoint remULCand[MAX_CELLS];
    OXPoint remLRCand[MAX_CELLS];

    OXPoint upperLeftCand, lowerRightCand;

    HashTable table;

    static int   RANK[107];
    static int   PRIOR[256][256];
    static UCHAR STATUS4[10][10][10][10];

    int totalSearched, nSearched;

    bool check();

    long start_time;
    long stopTime();
};

extern long getTime();

#endif

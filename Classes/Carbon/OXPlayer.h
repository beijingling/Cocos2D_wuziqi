/*
 *  AIPlayer.h
 */

#ifndef _OXPlayer
#define _OXPlayer

class OXPlayer
{
public:
    virtual void start(int size) = 0;
    virtual void move(int x, int y) = 0;
    virtual void yourTurn(int &x, int &y, int depth = 0, int time = 0) = 0;
    virtual void think() {};
    virtual void undo() = 0;
    virtual const char* name() const = 0;

    virtual ~OXPlayer() {};
};

void WriteLog(int points, int nSearched, int speed, int depth, bool debug);
#endif

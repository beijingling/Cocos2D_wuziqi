//
// Created by lava on 18-8-15.
//
#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "cocos2d.h"
#include "Carbon/OXTypes.h"
#include "Carbon/AICarbon.h"
class OXGame;

class ChessBoard :public cocos2d::Layer {
public:

    ChessBoard (void);
    ~ChessBoard (void);
    CREATE_FUNC(ChessBoard);

    virtual bool init() override ;
    void updateBoard(int x, int y, OXPiece piece);
    bool chessCallback(cocos2d::Touch *touch, cocos2d::Event *event);
    cocos2d::EventListener* createEventListener();
    void flashChess(cocos2d::Sprite *mySprite);
    void showWinLine(OXPoint *winPoint);

    cocos2d::Sprite* getChessByXY (int x, int y);
    void getXY(int &x, int &y, cocos2d::Sprite *node);
    void AIPlay();

    void setGame(OXGame *game)       {mOXGame = game;}
    void setAI(AICarbon* aiCarbon)   {mAICarbon = aiCarbon;}

    void setSize(int x, int y)       {Board_width = x; Board_height = y;}
private:
    float mScaleSize = 1.0;
    int Board_height = 15;
    int Board_width = 15;
    cocos2d::Vector<cocos2d::Sprite*> chess_nodes;
    OXGame* mOXGame;
    AICarbon* mAICarbon;
};
#endif
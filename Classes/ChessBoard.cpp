//
// Created by lava on 18-8-15.
//

#include"ChessBoard.h"
#include "ui/CocosGUI.h"
#include "Carbon/OXGame.h"
USING_NS_CC;

ChessBoard::ChessBoard (){}
ChessBoard::~ChessBoard (){}

bool ChessBoard::init() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // calculate each chess' size
    const float chessSize = visibleSize.width/Board_width;
    // get touch event Listener
    auto eventListener = createEventListener();
    //to create chess sprite object ;and set feature
    auto polygon = AutoPolygon::generatePolygon("yellow_empyt.png");
    Sprite* note = Sprite::create(polygon);
    mScaleSize = chessSize / note->getContentSize().height + 0.02/* avoid gap*/;

    float base_h = visibleSize.height/4;
    //to add chess sprite on board
    for (int j = Board_height; j > 0; j--) {
        for (int i = 0; i < Board_width; i++) {
            //polygon = AutoPolygon::generatePolygon("slice_0.png");
            note = Sprite::create(polygon);
            note->setScale(mScaleSize);
            float raw_height = chessSize * j - chessSize / 2;
            float raw_width = chessSize * i + chessSize / 2;
            note->setPosition(Vec2(raw_width + origin.x, raw_height + origin.y + base_h));
            //note->setRotation(i+j);
            //note->setAnchorPoint(Vec2(i/note_wid,j/note_heg));
            //note->setScale(note_heg / note->getContentSize().height);
            //note->setColor(Color3B(i%0xff,j%0xff,i+j%0xff));
            _eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener->clone(), note);
            chess_nodes.pushBack(note);
            this->addChild(note);
        }
    }
    return true;
}

void ChessBoard::updateBoard(int x, int y, OXPiece piece) {
    Sprite *target = getChessByXY(x, y);
    Texture2D* image = nullptr;
    if (piece == OP) {
        image = Director::getInstance()->getTextureCache()->addImage("yellow_black_normal.png");
    } else if (piece == XP) {
        flashChess(target);
        image = Director::getInstance()->getTextureCache()->addImage("yellow_white_normal.png");
    } else if (piece == EMPTY) {
        image = Director::getInstance()->getTextureCache()->addImage("yellow_empyt.png");
    } else {
        return;
    }
    log("updateBoard %d , %d, type:%d", x, y, piece);
    target->setTexture(image);
}


void ChessBoard::flashChess(Sprite *mySprite) {
    // to reset original scale
    mySprite->setScale(mScaleSize);
    // create a few Actions
    auto scale = ScaleBy::create(0.3f, 1.30f);
    auto delay = DelayTime::create(0.3f);

    auto sequence = Sequence::create(scale, delay, scale->reverse(),nullptr);

    // run it
    Action *act = mySprite->runAction(sequence);
    log("flashChess ");
}
void ChessBoard::showWinLine(OXPoint *winPoint){
    for (int i = 0; i < 5; ++i) {
        OXPoint point = *(winPoint + i);
        int sprite_index = point.y * Board_width + point.x;
        if (sprite_index > Board_height * Board_width) {
            sprite_index = 0;
        }
        auto target_sprite = chess_nodes.at(sprite_index);
        flashChess(target_sprite);
    }
}

void ChessBoard::getXY(int &x, int &y, Sprite *node) {
    int index = chess_nodes.getIndex(node);
    log("index %d", index);
    y = index / Board_width;
    x = index % Board_width;
}

Sprite* ChessBoard::getChessByXY (int x, int y) {
    int sprite_index = y * Board_width + x;
    if (sprite_index > Board_height * Board_width) {
        sprite_index = 0;
    }
    log("getChessByXY %d ",sprite_index);
    auto current_spr = chess_nodes.at(sprite_index);
    return current_spr;
}

bool ChessBoard::chessCallback(cocos2d::Touch *touch, cocos2d::Event *event) {

    auto currentSprite = static_cast<Sprite *>(event->getCurrentTarget());

    int x;
    int y;
    getXY(x, y, currentSprite);
    log("touch x:%d, y:%d", x, y);
    //check the point(x,y) have pressed
    bool checked = mOXGame->cell(x, y) != EMPTY;
    if (checked) {
        return true;
    }
    mOXGame->move(x, y);
    //invoc the AI
    AIPlay();
    return true;
}

void ChessBoard::AIPlay() {
    int x;
    int y;
    int depth = 0;
    int time = 10;

    //get the AI's calculated best point to play
    mAICarbon->yourTurn(x, y, depth, time);
    log("AI x:%d, y:%d", x, y);
    mOXGame->move(x,y);
}

EventListener *ChessBoard::createEventListener() {
    EventListenerTouchOneByOne *listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch *touch, Event *event) {
        auto currentSprite = static_cast<Sprite *>(event->getCurrentTarget());
        Point pos = Director::getInstance()->convertToGL(touch->getLocationInView());
        return currentSprite->getBoundingBox().containsPoint(pos);
    };
    listener->onTouchEnded = CC_CALLBACK_2(ChessBoard::chessCallback, this);
    return listener;
}
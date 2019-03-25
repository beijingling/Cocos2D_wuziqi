/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "MenuScene.h"

USING_NS_CC;

Scene *HelloWorld::createScene() {
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char *filename) {
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
            "ic_close_btn.png",
            "ic_delete_file_attachment.png",
            CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0) {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    } else {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2 - 5;
        float y = origin.y + closeItem->getContentSize().height / 2 + 5;
        closeItem->setPosition(Vec2(x, y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    //add restart button
    auto resetItem = MenuItemImage::create(
            "ic_reset.png",
            "ic_reset_pressed.png",
            CC_CALLBACK_1(HelloWorld::resetGame, this));
    auto reset_menu = Menu::create(resetItem, NULL);
    float rs_x = origin.x + visibleSize.width / 2;
    float rs_y = origin.y + resetItem->getContentSize().height / 2 + 5;
    reset_menu->setPosition(Vec2(rs_x, rs_y));
    this->addChild(reset_menu, 1);
    //add rollback button
    auto rollback = MenuItemImage::create("ic_maps_back.png", "ic_maps_back.png",
                                          CC_CALLBACK_1(HelloWorld::rollback, this));
    auto back_menu = Menu::create(rollback, NULL);
    rs_x = origin.x + visibleSize.width / 8;
    rs_y = origin.y + rollback->getContentSize().height / 2 + 5;
    back_menu->setPosition(Vec2(rs_x, rs_y));
    this->addChild(back_menu, 1);
    /////////////////////////////
    // init ai carbon
    g_carbon = new AICarbon();
    g_carbon->start(pan_width, pan_height);
    //init Game
    mOXGame = new OXGame();
    mOXGame->start(pan_width, pan_height);
    // init board
    mChessBoard = ChessBoard::create();
    mChessBoard->setSize(pan_width, pan_height);

    this->addChild(mChessBoard);

    mOXGame->setBoard(mChessBoard);
    mOXGame->setAI(g_carbon);
    mChessBoard->setGame(mOXGame);
    mChessBoard->setAI(g_carbon);

    //add bg
    //auto polygon = AutoPolygon::generatePolygon("bg.png");
    auto bg = Sprite::create("bg.png", Rect(0, 0, visibleSize.width, visibleSize.height));
    auto size = bg->getContentSize();
    bg->setPosition(Vec2(origin.x + size.width / 2, origin.y + size.height / 2));
    this->addChild(bg, -10);
    return true;
}


void HelloWorld::menuCloseCallback(Ref *pSender) {
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::rollback(Ref *pSender) {
    log("rollback");
    mOXGame->undo();
}


void HelloWorld::resetGame(Ref *pSender) {
    log("resetGame");

    mOXGame->start(pan_width, pan_height);
    //mSteps.clear();
    g_carbon->start(pan_width, pan_height);
}

void HelloWorld::showParticle() {

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto star_particle = ParticleSystem::create("particle_exture.plist");
    star_particle->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    auto *size = new Size(20, 20);
    star_particle->setScale(0.1f);
    this->addChild(star_particle, 20);
    star_particle->start();
}


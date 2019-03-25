//
// Created by lava on 18-6-9.
//

#include "MenuScene.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

/*Scene* MenuScene::createScene(){
    return MenuScene::create();
}*/

bool MenuScene::init() {

    if(!Scene::init()){
        return false;
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    //Vector<MenuItem*> MenuItems ;
    auto label = Label::createWithTTF("this is a menu", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        //problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    return true;
}
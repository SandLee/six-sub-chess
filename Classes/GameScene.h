#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"

class GameLogic;
class SimpleRobot;

class GameScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
	CREATE_FUNC(GameScene);

private:
	virtual void update(float delta) override;

private:
	std::auto_ptr<GameLogic> logic_;
	std::auto_ptr<SimpleRobot> robot_;
};

#endif
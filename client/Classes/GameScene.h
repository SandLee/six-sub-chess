#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"
#include "SingleLogic.h"
#include "SimpleRobot.h"

class CheckerboardLayer;

class GameScene : public cocos2d::Layer
{
	static const int kMenuItemInterval = 5;		// 子菜单间距
	static const int kMenuItemWidth = 140;		// 子菜单宽度
	static const int kMenuItemHeight = 100;		// 子菜单高度

public:
	GameScene();

	~GameScene();

    static cocos2d::Scene* createScene();

    virtual bool init();
    
	CREATE_FUNC(GameScene);

public:
	void setGameTips(const std::string &str);

public:
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

private:
	void startGame();

	void onBeReady();

	virtual void update(float delta) override;

	virtual void onEnterTransitionDidFinish() override;

private:
	CheckerboardLayer*			checkerboard_;
	cocos2d::Label*				game_tips_;
	cocos2d::Node*				selected_item_;
	std::auto_ptr<SingleLogic>	logic_;
	std::auto_ptr<SimpleRobot>	robot_;
};

#endif
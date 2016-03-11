#ifndef __WELCOMESCENE_H__
#define __WELCOMESCENE_H__

#include "cocos2d.h"

class WelcomeScene : public cocos2d::LayerColor
{
	static const int kMenuRowNum = 2;			// 子菜单行数
	static const int kMenuColNum = 2;			// 子菜单列数
	static const int kMenuItemInterval = 5;		// 子菜单间距
	static const int kMenuItemWidth = 250;		// 子菜单宽度
	static const int kMenuItemHeight = 250;		// 子菜单高度

public:
	WelcomeScene();

	~WelcomeScene();

	static cocos2d::Scene* createScene();

	virtual bool init() override;

	CREATE_FUNC(WelcomeScene);

public:
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

private:
	cocos2d::Node* selected_item_;
};

#endif
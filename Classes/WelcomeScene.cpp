#include "WelcomeScene.h"

#include "VisibleRect.h"
#include "ColorGenerator.h"
using namespace cocos2d;


Scene* WelcomeScene::createScene()
{
	auto scene = Scene::create();
	auto layer = WelcomeScene::create();
	scene->addChild(layer);
	return scene;
}

bool WelcomeScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	// 创建菜单
	Vec2 start_pos;
	start_pos.x = VisibleRect::center().x - (kMenuItemWidth * kMenuColNum + kMenuItemInterval * (kMenuColNum - 1)) / 2 + kMenuItemWidth / 2;
	start_pos.y = VisibleRect::center().y - (kMenuItemHeight * kMenuRowNum + kMenuItemInterval * (kMenuRowNum - 1)) / 2 + kMenuItemHeight / 2;
	std::vector<cocos2d::Color3B> colors = ColorGenerator::instance()->rand_not_repeat(5);
	for (size_t i = 0; i < 4; ++i)
	{
		int row = i / kMenuColNum;
		int col = i % kMenuColNum;
		auto layer = LayerColor::create();
		layer->setAnchorPoint(Vec2(0.5f, 0.5f));
		layer->initWithColor(Color4B(colors[i]));
		layer->setContentSize(Size(kMenuItemWidth, kMenuItemHeight));
		layer->ignoreAnchorPointForPosition(false);
		layer->setPosition(start_pos + Vec2(col * kMenuItemWidth + col * kMenuItemInterval, row * kMenuItemHeight + row * kMenuItemInterval));
		addChild(layer);
	}

	initWithColor(Color4B(colors.back()));

	return true;
}
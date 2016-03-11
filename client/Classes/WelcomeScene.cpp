#include "WelcomeScene.h"

#include <array>
#include "Language.h"
#include "GameScene.h"
#include "VisibleRect.h"
#include "ColorGenerator.h"
using namespace cocos2d;


enum MenuItemType
{
	SingleGame = 1,
	OnlineGame,
	About,
	QuitGame,
};

WelcomeScene::WelcomeScene()
	: selected_item_(nullptr)
{

}

WelcomeScene::~WelcomeScene()
{

}

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

	// 设置语言
	Language::instance()->set_language(LanguageType::CHINESE);

	// 创建菜单
	Vec2 start_pos;
	std::array<MenuItemType, 4> tags = { SingleGame, OnlineGame, About, QuitGame };
	std::array<const char*, 4> menu_texts = { "single_game", "online_game", "about", "quit_game" };
	auto yyy = VisibleRect::center().y;
	start_pos.x = VisibleRect::center().x - (kMenuItemWidth * kMenuColNum + kMenuItemInterval * (kMenuColNum - 1)) / 2 + kMenuItemWidth / 2;
	start_pos.y = VisibleRect::center().y + (kMenuItemHeight * kMenuRowNum + kMenuItemInterval * (kMenuRowNum - 1)) / 2 - kMenuItemHeight / 2;
	std::vector<cocos2d::Color3B> colors = ColorGenerator::instance()->rand_not_repeat(5);
	for (size_t i = 0; i < 4; ++i)
	{
		int row = i / kMenuColNum;
		int col = i % kMenuColNum;
		auto menu_item = LayerColor::create();
		menu_item->setAnchorPoint(Vec2(0.5f, 0.5f));
		menu_item->initWithColor(Color4B(colors[i]));
		menu_item->setContentSize(Size(kMenuItemWidth, kMenuItemHeight));
		menu_item->ignoreAnchorPointForPosition(false);
		menu_item->setPosition(start_pos + Vec2(col * kMenuItemWidth + col * kMenuItemInterval, -(row * kMenuItemHeight + row * kMenuItemInterval)));
		menu_item->setTag(tags[i]);
		addChild(menu_item);

		auto size = menu_item->getContentSize();
		auto label = Label::createWithSystemFont(lang(menu_texts[i]).c_str(), "", 32);
		label->setColor(Color3B(0, 0, 0));
		label->setAnchorPoint(Vec2(0.0f, 0.5f));
		label->setPosition(Vec2(size.width / 6, size.height / 4));
		menu_item->addChild(label);
	}
	initWithColor(Color4B(colors.back()));

	// 监听触摸事件
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(WelcomeScene::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(WelcomeScene::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(WelcomeScene::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(WelcomeScene::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

bool WelcomeScene::onTouchBegan(Touch *touch, Event *unused_event)
{
	for (int i = SingleGame; i <= QuitGame; ++i)
	{
		if (getChildByTag(i)->getBoundingBox().containsPoint(touch->getLocation()))
		{
			selected_item_ = getChildByTag(i);
			selected_item_->setOpacity(155);
			return true;
		}
	}
	return false;
}

void WelcomeScene::onTouchMoved(Touch *touch, Event *unused_event)
{

}

void WelcomeScene::onTouchEnded(Touch *touch, Event *unused_event)
{
	for (int i = SingleGame; i <= QuitGame; ++i)
	{
		if (getChildByTag(i)->getBoundingBox().containsPoint(touch->getLocation()))
		{
			if (getChildByTag(i) == selected_item_)
			{
				switch ((MenuItemType)i)
				{
					case SingleGame:
					case OnlineGame:
					{
						runAction(Sequence::create(
							DelayTime::create(0.2f),
							CallFunc::create([=]()
						{
							Director::getInstance()->replaceScene(TransitionFade::create(0.5f, GameScene::createScene()));
						}),
							nullptr));
						break;
					}
					case About:
					{
						Application::getInstance()->openURL("https://github.com/zhangpanyi/six-sub-chess");
						break;
					}
					case QuitGame:
					{
						runAction(Sequence::create(
							DelayTime::create(0.2f),
							CallFunc::create([=]()
						{
							Director::getInstance()->end();
						}),
							nullptr));
						break;
					}
				}
			}
			break;
		}
	}
	selected_item_->setOpacity(255);
	selected_item_ = nullptr;
}

void WelcomeScene::onTouchCancelled(Touch *touch, Event *unused_event)
{
	onTouchEnded(touch, unused_event);
}
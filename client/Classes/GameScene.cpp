#include "GameScene.h"

#include <numeric>
#include "Language.h"
#include "VisibleRect.h"
#include "WelcomeScene.h"
#include "ColorGenerator.h"
#include "CheckerboardLayer.h"
using namespace cocos2d;


enum MenuItemType
{
	Restart = 1,
	GotoMainMenu,
};

GameScene::GameScene()
	: checkerboard_(nullptr)
	, game_tips_(nullptr)
	, selected_item_(nullptr)
{

}

GameScene::~GameScene()
{

}

Scene* GameScene::createScene()
{
    auto scene = Scene::create();
	auto layer = GameScene::create();
    scene->addChild(layer);
    return scene;
}

bool GameScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }

	// 创建菜单
	std::array<MenuItemType, 2> tags = { Restart, GotoMainMenu };
	std::array<const char*, 2> menu_texts = { "restart", "mainmenu" };
	float start_x = VisibleRect::rightBottom().x - kMenuItemWidth * 2 - kMenuItemInterval * 2;
	for (int i = 0; i < 2; ++i)
	{
		auto menu_item = LayerColor::create();
		menu_item->setAnchorPoint(Vec2(0.0f, 0.0f));
		menu_item->initWithColor(Color4B(ColorGenerator::instance()->rand()));
		menu_item->setContentSize(Size(kMenuItemWidth, kMenuItemHeight));
		menu_item->ignoreAnchorPointForPosition(false);
		menu_item->setPosition(Vec2(start_x + i * kMenuItemWidth + i * kMenuItemInterval, VisibleRect::rightBottom().y + kMenuItemInterval));
		menu_item->setTag(tags[i]);
		addChild(menu_item);

		auto size = menu_item->getContentSize();
		auto label = Label::createWithSystemFont(lang(menu_texts[i]).c_str(), "", 24);
		label->setColor(Color3B(0, 0, 0));
		label->setAnchorPoint(Vec2(0.5f, 0.5f));
		label->setPosition(Vec2(size.width / 2, size.height / 2));
		menu_item->addChild(label);
	}

	// 玩家操作图层
	logic_.reset(new SingleLogic());
	robot_.reset(new SimpleRobot(logic_.get()));
	checkerboard_ = CheckerboardLayer::create(logic_.get());
	addChild(checkerboard_, 1);

	// 游戏提示
	game_tips_ = Label::createWithSystemFont("", "", 48);
	game_tips_->setColor(Color3B(255, 0, 0));
	game_tips_->setAnchorPoint(Vec2(0.5f, 0.5f));
	game_tips_->setPosition(VisibleRect::top() - Vec2(0, 150));
	addChild(game_tips_);

	// 开启触摸
	auto menu_touch_layer = Layer::create();
	addChild(menu_touch_layer, 2);
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(GameScene::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, menu_touch_layer);

	// 监听游戏逻辑
	logic_->add_action_update_callback(std::bind(&GameScene::onBeReady, this));
	
	scheduleUpdate();

    return true;
}

void GameScene::setGameTips(const std::string &str)
{
	game_tips_->setString(str);
}

// 开始游戏
void GameScene::startGame()
{
	logic_->ready();
}

// 准备完毕
void GameScene::onBeReady()
{
	FAction action = logic_->get_action_from_queue(logic_->get_action_num() - 1);
	if (action.type == FActionType::BEREADY)
	{
		robot_->reset(logic_->get_upperplayer_chesspiece_type());
		checkerboard_->reset(logic_->get_belowplayer_chesspiece_type());
	}	
}

void GameScene::update(float delta)
{
	if (logic_.get())
	{
		logic_->update(delta);
	}
}

void GameScene::onEnterTransitionDidFinish()
{
	startGame();
}

bool GameScene::onTouchBegan(Touch *touch, Event *unused_event)
{
	for (int i = Restart; i <= GotoMainMenu; ++i)
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

void GameScene::onTouchMoved(Touch *touch, Event *unused_event)
{

}

void GameScene::onTouchEnded(Touch *touch, Event *unused_event)
{
	for (int i = Restart; i <= GotoMainMenu; ++i)
	{
		if (getChildByTag(i)->getBoundingBox().containsPoint(touch->getLocation()))
		{
			if (getChildByTag(i) == selected_item_)
			{
				switch ((MenuItemType)i)
				{
					case Restart:
					{
						startGame();
						break;
					}
					case GotoMainMenu:
					{
						runAction(Sequence::create(
							DelayTime::create(0.2f),
							CallFunc::create([=]()
						{
							Director::getInstance()->replaceScene(TransitionFade::create(0.5f, WelcomeScene::createScene()));
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

void GameScene::onTouchCancelled(Touch *touch, Event *unused_event)
{
	onTouchCancelled(touch, unused_event);
}
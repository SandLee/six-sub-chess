#include "GameScene.h"

#include <numeric>
#include "Language.h"
#include "VisibleRect.h"
#include "WelcomeScene.h"
#include "json/document.h"
#include "ColorGenerator.h"
#include "CheckerboardLayer.h"
using namespace cocos2d;


enum MenuItemType
{
	Restart = 1,
	GotoMainMenu,
};

// 初始化棋盘
void InitCheckerboard(GameLogic::ChessArray &checkerboard)
{
	Data data = FileUtils::getInstance()->getDataFromFile("config/init.json");

	// 解析json
	rapidjson::Document doc;
	doc.Parse<0>(std::string((const char *)data.getBytes(), data.getSize()).c_str());
	if (doc.HasParseError() || !doc.IsArray())
	{
		CCAssert(false, "Json parse error!");
	}

	if (doc.Size() != GameLogic::kCheckerboardRowNum * GameLogic::kCheckerboardColNum)
	{
		CCAssert(false, "Array size error!");
	}

	for (size_t i = 0; i < doc.Size(); ++i)
	{
		int row = i / GameLogic::kCheckerboardColNum;
		int col = i % GameLogic::kCheckerboardColNum;
		checkerboard[row * GameLogic::kCheckerboardRowNum + col] = static_cast<GameLogic::ChessPieceType>(doc[doc.Size() - i - 1].GetInt());
	}
}

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
	logic_.reset(new GameLogic());
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
	
	scheduleUpdate();

    return true;
}

void GameScene::setGameTips(const std::string &str)
{
	game_tips_->setString(str);
}

void GameScene::start_game()
{
	setGameTips("");
	GameLogic::ChessArray checkerboard;
	InitCheckerboard(checkerboard);

	if ((rand() % 2) == 0)
	{
		robot_->reset(GameLogic::WHITE);
		checkerboard_->reset(GameLogic::BLACK);
	}
	else
	{
		robot_->reset(GameLogic::BLACK);
		checkerboard_->reset(GameLogic::WHITE);
	}
	logic_->start(checkerboard);
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
	start_game();
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
						start_game();
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
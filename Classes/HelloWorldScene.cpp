#include "HelloWorldScene.h"
#include "CheckerboardLayer.h"
#include "GameLogic.h"

USING_NS_CC;


#include "GameLogic.h"

#include <numeric>
#include "json/document.h"
using namespace cocos2d;



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

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

	GameLogic::ChessArray checkerboard;
	InitCheckerboard(checkerboard);
	GameLogic *logic = new GameLogic(checkerboard); // 测试
	auto p = CheckerboardLayer::create();
	p->generate_chessboard(GameLogic::ChessPieceType::WHITE, logic);
	addChild(p);
    
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

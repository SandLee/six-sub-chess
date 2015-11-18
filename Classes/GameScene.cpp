#include "GameScene.h"

#include <numeric>
#include "json/document.h"
#include "CheckerboardLayer.h"

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

	// 初始化棋盘
	GameLogic::ChessArray checkerboard;
	InitCheckerboard(checkerboard);

	// 创建菜单


	// 操作图层
	logic_.reset(new GameLogic(checkerboard));
	robot_.reset(new SimpleRobot(GameLogic::ChessPieceType::BLACK, logic_.get()));
	auto layer = CheckerboardLayer::create();
	layer->generate_chessboard(GameLogic::ChessPieceType::WHITE, logic_.get());
	addChild(layer, 1);

	scheduleUpdate();

    return true;
}

void GameScene::update(float delta)
{
	if (logic_.get())
	{
		logic_->update(delta);
	}
}
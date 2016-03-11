﻿#include "SingleLogic.h"
#include <cmath>
#include <cassert>
#include "cocos2d.h"
#include "json/document.h"
using namespace cocos2d;

namespace
{
	// 初始化棋盘
	void InitCheckerboard(FChessArray &checkerboard)
	{
		Data data = FileUtils::getInstance()->getDataFromFile("config/init.json");

		// 解析json
		rapidjson::Document doc;
		doc.Parse<0>(std::string((const char *)data.getBytes(), data.getSize()).c_str());
		if (doc.HasParseError() || !doc.IsArray())
		{
			CCAssert(false, "Json parse error!");
		}

		if (doc.Size() != kCheckerboardRowNum * kCheckerboardColNum)
		{
			CCAssert(false, "Array size error!");
		}

		for (size_t i = 0; i < doc.Size(); ++i)
		{
			int row = i / kCheckerboardColNum;
			int col = i % kCheckerboardColNum;
			checkerboard[row * kCheckerboardRowNum + col] = static_cast<FChessPieceType>(doc[doc.Size() - i - 1].GetInt());
		}
	}
}

// 准备开始
void SingleLogic::ready()
{
	reset();
	FChessArray checkerboard;
	InitCheckerboard(checkerboard);
	set_checkerboard(checkerboard);
	upperplayer_ = rand() % 2 == 0 ? FChessPieceType::WHITE : FChessPieceType::BLACK;
	add_action(FActionType::BEREADY, get_standby_chesspiece_type(), FVec2::invalid(), FVec2::invalid());
	add_action(FActionType::START, get_standby_chesspiece_type(), FVec2::invalid(), FVec2::invalid());
	add_action(FActionType::STANDBY, get_standby_chesspiece_type(), FVec2::invalid(), FVec2::invalid());
}

// 获取上方玩家棋子类型
FChessPieceType SingleLogic::get_upperplayer_chesspiece_type() const
{
	return upperplayer_;
}

// 获取下方玩家棋子类型
FChessPieceType SingleLogic::get_belowplayer_chesspiece_type() const
{
	return upperplayer_ == FChessPieceType::WHITE ? FChessPieceType::BLACK : FChessPieceType::WHITE;
}

// 移动棋子
void SingleLogic::move_chesspiece(const FVec2 &source, const FVec2 &target)
{
	FMoveTrack track = { source, target };
	add_move_track(track);
}
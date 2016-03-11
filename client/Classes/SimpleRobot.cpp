#include "SimpleRobot.h"

#include <ctime>
#include <random>
#include <cassert>
#include <algorithm>


SimpleRobot::SimpleRobot(SingleLogic *logic)
	: logic_(logic)
	, action_read_pos_(0)
	, chess_type_(FChessPieceType::NONE)
{
	assert(logic_ != nullptr);
	logic_->addActionUpdateCallback(std::bind(&SimpleRobot::updateAction, this));
}

SimpleRobot::~SimpleRobot()
{

}

// 更新动作
void SimpleRobot::updateAction()
{
	runAction();
}

// 获取所有可行的移动路径
std::vector<FMoveTrack> SimpleRobot::getAllMovetrack(const FChessArray &checkerboard, FChessPieceType type) const
{
	size_t idx = 0;
	std::vector<FMoveTrack> track_array;
	while (idx != checkerboard.size())
	{
		if (checkerboard[idx] == type)
		{
			// 判断上下左右是否可移动
			int row = idx / kCheckerboardColNum;
			int col = idx % kCheckerboardColNum;
			for (int i = -1; i <= 1; i += 2)
			{
				FVec2 v1(col + i, row);
				FVec2 v2(col, row + i);
				if (isInCheckerboard(v1) && !isValidChesspiece(checkerboard, v1))
				{
					FMoveTrack track = { FVec2(col, row), v1 };
					track_array.push_back(track);
				}
				if (isInCheckerboard(v2) && !isValidChesspiece(checkerboard, v2))
				{
					FMoveTrack track = { FVec2(col, row), v2 };
					track_array.push_back(track);
				}
			}
		}
		++idx;
	}
	return track_array;
}

// 获取可杀死敌方棋子的移动路径
std::vector<FMoveTrack> SimpleRobot::getCanKillChessMovetrack(std::vector<FMoveTrack> &track_array) const
{
	std::vector<FMoveTrack> kill_chess_array;
	FChessArray checkerboard = logic_->getCheckerboard();
	for (size_t i = 0; i < track_array.size(); ++i)
	{
		// 模拟出棋
		FVec2 &source =  track_array[i].source;
		FVec2 &target = track_array[i].target;
		std::swap(checkerboard[source.y * kCheckerboardColNum + source.x], checkerboard[target.y * kCheckerboardColNum + target.x]);
		
		std::set<FVec2> kill_set = helper::CheckKillChesspiece(checkerboard, target);
		if (!kill_set.empty())
		{
			kill_chess_array.push_back(track_array[i]);
		}

		// 恢复棋盘
		std::swap(checkerboard[source.y * kCheckerboardColNum + source.x], checkerboard[target.y * kCheckerboardColNum + target.x]);
	}

	return kill_chess_array;
}

// 获取可躲避被杀棋的移动路径
std::vector<FMoveTrack> SimpleRobot::getCanAvoidChessMovetrack(std::vector<FMoveTrack> &track_array) const
{
	std::vector<FMoveTrack> avoid_chess_array = track_array;
	FChessArray checkerboard = logic_->getCheckerboard();
	auto other_chesspiece_type = getChesspieceType() == FChessPieceType::WHITE ? FChessPieceType::BLACK : FChessPieceType::WHITE;
	for (size_t i = 0; i < track_array.size(); ++i)
	{
		// 模拟出棋
		FVec2 &source = track_array[i].source;
		FVec2 &target = track_array[i].target;
		std::swap(checkerboard[source.y * kCheckerboardColNum + source.x], checkerboard[target.y * kCheckerboardColNum + target.x]);

		// 模拟对方出棋
		{
			std::vector<FMoveTrack> other_track_array = getAllMovetrack(checkerboard, other_chesspiece_type);
			for (size_t j = 0; j < other_track_array.size(); ++j)
			{
				// 模拟出棋
				FVec2 &other_source = other_track_array[j].source;
				FVec2 &other_target = other_track_array[j].target;
				std::swap(checkerboard[other_source.y * kCheckerboardColNum + other_source.x], checkerboard[other_target.y * kCheckerboardColNum + other_target.x]);

				if (!helper::CheckKillChesspiece(checkerboard, other_target).empty())
				{
					auto itr = std::find(avoid_chess_array.begin(), avoid_chess_array.end(), track_array[i]);
					if (itr != avoid_chess_array.end())
					{
						avoid_chess_array.erase(itr);
					}
				}

				// 恢复棋盘
				std::swap(checkerboard[other_source.y * kCheckerboardColNum + other_source.x], checkerboard[other_target.y * kCheckerboardColNum + other_target.x]);
			}
		}

		// 恢复棋盘
		std::swap(checkerboard[source.y * kCheckerboardColNum + source.x], checkerboard[target.y * kCheckerboardColNum + target.x]);
	}

	return avoid_chess_array;
}

// 执行动作
void SimpleRobot::runAction()
{
	FAction action = logic_->getActionFromQueue(action_read_pos_);
	if (action.type != FActionType::NONE)
	{
		if (action.type == FActionType::STANDBY &&
			action.chess_type != getChesspieceType())
		{
			// 获取所有可行的移动轨迹
			const FChessArray &checkerboard = logic_->getCheckerboard();
			std::vector<FMoveTrack> track_array = getAllMovetrack(checkerboard, getChesspieceType());

			if (!track_array.empty())
			{
				// 获取可杀死敌方棋子的移动轨迹
				std::vector<FMoveTrack> kill_chess_array = getCanKillChessMovetrack(track_array);

				// 优先杀死对方棋子，其次躲避对方
				if (!kill_chess_array.empty())
				{
					logic_->moveChesspiece(kill_chess_array.front().source, kill_chess_array.front().target);
				}
				else
				{
					std::vector<FMoveTrack> avoid_chess_array = getCanAvoidChessMovetrack(track_array);
					if (avoid_chess_array.empty())
					{
						std::default_random_engine generator(time(nullptr));
						std::uniform_int_distribution<int> dis(0, track_array.size() - 1);
						const FMoveTrack &track = track_array[dis(generator)];
						logic_->moveChesspiece(track.source, track.target);
					}
					else
					{
						std::default_random_engine generator(time(nullptr));
						std::uniform_int_distribution<int> dis(0, avoid_chess_array.size() - 1);
						const FMoveTrack &track = avoid_chess_array[dis(generator)];
						logic_->moveChesspiece(track.source, track.target);
					}
				}
			}
		}

		actionFinished();
	}
}

// 完成动作
void SimpleRobot::actionFinished()
{
	++action_read_pos_;
	updateAction();
}

// 重置
void SimpleRobot::reset(FChessPieceType type)
{
	chess_type_ = type;
	action_read_pos_ = 0;
}

// 获取棋子类型
FChessPieceType SimpleRobot::getChesspieceType() const
{
	return chess_type_;
}
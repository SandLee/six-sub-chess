#include "LogicBase.h"
#include <cmath>
#include <cassert>


LogicBase::LogicBase()
	: standby_chess_type_(FChessPieceType::BLACK)
{
	for (size_t i = 0; i < checkerboard_.size(); ++i)
	{
		checkerboard_[i] = FChessPieceType::NONE;
	}
}

LogicBase::~LogicBase()
{

}

// 重置
void LogicBase::reset()
{
	action_queue_.clear();
	standby_chess_type_ = FChessPieceType::BLACK;

	while (!move_queue_.empty())
	{
		move_queue_.pop();
	}

	for (size_t i = 0; i < checkerboard_.size(); ++i)
	{
		checkerboard_[i] = FChessPieceType::NONE;
	}
}

// 设置棋盘
void LogicBase::setCheckerboard(const FChessArray &checkerboard)
{
	checkerboard_ = checkerboard;
}

// 添加移动轨迹
void LogicBase::addMovetrack(const FMoveTrack &track)
{
	move_queue_.push(track);
}

// 获取动作数量
size_t LogicBase::getActionNum() const
{
	return action_queue_.size();
}

// 取出动作信息
FAction LogicBase::getActionFromQueue(size_t index)
{
	FAction action;
	action.type = FActionType::NONE;
	action.chess_type = FChessPieceType::NONE;
	if (index < action_queue_.size())
	{
		action = action_queue_[index];
	}
	return action;
}

// 添加动作更新通知
void LogicBase::addActionUpdateCallback(const std::function<void()> &callback)
{
	assert(callback != nullptr);
	if (callback != nullptr)
	{
		action_callback_list_.push_back(callback);
	}
}

// 获取棋盘数据
const FChessArray& LogicBase::getCheckerboard() const
{
	return checkerboard_;
}

// 浏览棋盘
void LogicBase::visitCheckerboard(const std::function<void(const FVec2&, FChessPieceType type)> &callback)
{
	if (callback != nullptr)
	{
		for (size_t i = 0; i < checkerboard_.size(); ++i)
		{
			int row = i / kCheckerboardColNum;
			int col = i % kCheckerboardColNum;
			callback(FVec2(col, row), checkerboard_[i]);
		}
	}
}

// 添加动作
void LogicBase::addAction(FActionType type, FChessPieceType chess_type, const FVec2 &source, const FVec2 &target)
{
	FAction action;
	action.type = type;
	action.source = source;
	action.target = target;
	action.chess_type = chess_type;
	action_queue_.push_back(action);
	for (auto &func : action_callback_list_)
	{
		func();
	}
}

// 是否在棋盘
bool LogicBase::isInCheckerboard(const FVec2 &pos) const
{
	return pos.x >= 0 && pos.y >= 0 && pos.x < kCheckerboardColNum && pos.y < kCheckerboardRowNum;
}

// 棋子是否有效
bool LogicBase::isValidChesspiece(const FVec2 &pos) const
{
	return isInCheckerboard(pos) && checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] != FChessPieceType::NONE;
}

// 获取棋子类型
FChessPieceType LogicBase::getChesspieceType(const FVec2 &pos) const
{
	return isValidChesspiece(pos) ? checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] : FChessPieceType::NONE;
}

// 获取待机棋子类型
FChessPieceType LogicBase::getStandbyChesspieceType() const
{
	return standby_chess_type_;
}

// 是否相邻
bool LogicBase::isAdjacent(const FVec2 &a, const FVec2 &b) const
{
	return isInCheckerboard(a) && isInCheckerboard(a) && (std::abs(a.x + a.y - b.x - b.y) == 1);
}

// 获取所有可行的移动路径
std::vector<FMoveTrack> LogicBase::getAllMovetrack(FChessPieceType type) const
{
	size_t idx = 0;
	std::vector<FMoveTrack> track_array;
	while (idx != checkerboard_.size())
	{
		if (checkerboard_[idx] == type)
		{
			// 判断上下左右是否可移动
			int row = idx / kCheckerboardColNum;
			int col = idx % kCheckerboardColNum;
			for (int i = -1; i <= 1; i += 2)
			{
				FVec2 v1(col + i, row);
				FVec2 v2(col, row + i);
				if (isInCheckerboard(v1) && !isValidChesspiece(v1))
				{
					FMoveTrack track = { FVec2(col, row), v1 };
					track_array.push_back(track);
				}
				if (isInCheckerboard(v2) && !isValidChesspiece(v2))
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

// 更新
void LogicBase::update(float dt)
{
	while (!move_queue_.empty())
	{
		const FVec2 &source = move_queue_.front().source;
		const FVec2 &target = move_queue_.front().target;

		if (source != target && isValidChesspiece(source) && !isValidChesspiece(target)
			&& isAdjacent(source, target) && checkerboard_[target.y  * kCheckerboardRowNum + target.x] != standby_chess_type_)
		{
			// 交换数据
			std::swap(checkerboard_[source.y  * kCheckerboardRowNum + source.x], checkerboard_[target.y  * kCheckerboardRowNum + target.x]);

			// 新增动作
			addAction(FActionType::MOVED, checkerboard_[target.y  * kCheckerboardRowNum + target.x], source, target);

			// 检测杀棋
			std::set<FVec2> killed_set = helper::CheckKillChesspiece(checkerboard_, target);
			for (auto &pos : killed_set)
			{
				checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] = FChessPieceType::NONE;
				addAction(FActionType::KILLED, FChessPieceType::NONE, target, pos);
			}

			// 是否无棋可用
			int count = 0;
			standby_chess_type_ = checkerboard_[target.y  * kCheckerboardRowNum + target.x];
			auto other_chess_type = standby_chess_type_ == FChessPieceType::WHITE ? FChessPieceType::BLACK : FChessPieceType::WHITE;
			for (size_t i = 0; i < checkerboard_.size(); ++i)
			{
				if (checkerboard_[i] == other_chess_type)
				{
					++count;
				}
			}

			// 游戏是否结束
			if (count <= 1)
			{
				addAction(FActionType::GAMEOVER, standby_chess_type_, FVec2::invalid(), FVec2::invalid());
			}
			else
			{
				// 玩家待机	
				if (!getAllMovetrack(other_chess_type).empty())
				{
					addAction(FActionType::STANDBY, standby_chess_type_, FVec2::invalid(), FVec2::invalid());
				}
				else
				{
					// 山穷水尽
					addAction(FActionType::GAMEOVER, standby_chess_type_, FVec2::invalid(), FVec2::invalid());
					for (size_t i = 0; i < checkerboard_.size(); ++i)
					{
						if (checkerboard_[i] == other_chess_type)
						{
							int row = i / kCheckerboardColNum;
							int col = i % kCheckerboardColNum;
							addAction(FActionType::KILLED, FChessPieceType::NONE, FVec2::invalid(), FVec2(col, row));
						}
					}
				}
			}
		}

		move_queue_.pop();
	}
}

namespace helper
{
	// 获取横向相连的棋子
	std::vector<FVec2> GetChesspiecesWithHorizontal(const FChessArray &checkerboard, const FVec2 &pos)
	{
		int continuous = 0;
		std::vector<FVec2> ret;
		int last_type = checkerboard[pos.y * kCheckerboardColNum];
		for (int i = 0; i < kCheckerboardColNum; ++i)
		{
			int idx = pos.y * kCheckerboardColNum + i;
			if (checkerboard[idx] != FChessPieceType::NONE)
			{
				++continuous;
				ret.push_back(FVec2(i, pos.y));
			}
			else
			{
				if (continuous == 3)
				{
					break;
				}
				else
				{
					continuous = 0;
					ret.clear();
				}
			}
		}

		if (ret.size() != 3)
		{
			ret.clear();
		}

		return ret;
	}

	// 获取纵向相连的棋子
	std::vector<FVec2> GetChesspiecesWithVertical(const FChessArray &checkerboard, const FVec2 &pos)
	{
		int continuous = 0;
		std::vector<FVec2> ret;
		int last_type = checkerboard[pos.y * kCheckerboardColNum];
		for (int i = 0; i < kCheckerboardRowNum; ++i)
		{
			int idx = i * kCheckerboardColNum + pos.x;
			if (checkerboard[idx] != FChessPieceType::NONE)
			{
				++continuous;
				ret.push_back(FVec2(pos.x, i));
			}
			else
			{
				if (continuous == 3)
				{
					break;
				}
				else
				{
					continuous = 0;
					ret.clear();
				}
			}
		}

		if (ret.size() != 3)
		{
			ret.clear();
		}

		return ret;
	}

	// 获取可杀死的棋子
	std::set<FVec2> GetKilledChesspiece(const FChessArray &checkerboard, FChessPieceType key, const std::vector<FVec2> &chesspieces)
	{
		std::set<FVec2> ret;
		if (chesspieces.size() == 3)
		{
			size_t sum = std::accumulate(chesspieces.begin(), chesspieces.end(), 0, [=](size_t sum, const FVec2 &item)->size_t
			{
				if (key == checkerboard[item.y * kCheckerboardColNum + item.x])
				{
					return sum + 1;
				}
				return sum;
			});

			if (sum == 2)
			{
				for (size_t i = 0; i < chesspieces.size(); ++i)
				{
					int index = chesspieces[i].y * kCheckerboardColNum + chesspieces[i].x;
					if (i != 1 && checkerboard[index] != key)
					{
						ret.insert(chesspieces[i]);
					}
				}
			}
		}

		return ret;
	}

	// 检查可吃掉的棋子
	std::set<FVec2> CheckKillChesspiece(const FChessArray &checkerboard, const FVec2 &pos)
	{
		auto key = checkerboard[pos.y * kCheckerboardColNum + pos.x];
		auto v_array = GetChesspiecesWithVertical(checkerboard, pos);
		auto h_array = GetChesspiecesWithHorizontal(checkerboard, pos);
		auto horizontal_set = GetKilledChesspiece(checkerboard, key, h_array);
		auto vertical_set = GetKilledChesspiece(checkerboard, key, v_array);
		horizontal_set.insert(vertical_set.begin(), vertical_set.end());
		return horizontal_set;
	}
}
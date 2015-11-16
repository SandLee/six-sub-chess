#include "GameLogic.h"
#include <cassert>

GameLogic::GameLogic(const ChessArray &checkerboard)
	: lock_(false)
	, checkerboard_(checkerboard)
{

}

GameLogic::~GameLogic()
{

}

// 取出动作信息
GameLogic::Action GameLogic::take_action_from_queue()
{
	Action action;
	action.type = ActionType::NONE;
	action.chess_type = ChessPieceType::NONE;
	if (!action_queue_.empty())
	{
		action = action_queue_.front();
		action_queue_.pop();
	}
	return action;
}

// 添加动作更新通知
void GameLogic::add_action_update_callback(std::function<void()> &&callback)
{
	action_callback_ = callback;
}

// 获取棋盘数据
const GameLogic::ChessArray& GameLogic::get_checkerboard() const
{
	return checkerboard_;
}

// 浏览棋盘
void GameLogic::visit_checkerboard(const std::function<void(const Vec2&, ChessPieceType type)> &callback)
{
	if (callback != nullptr)
	{
		for (size_t i = 0; i < checkerboard_.size(); ++i)
		{
			int row = i / kCheckerboardColNum;
			int col = i % kCheckerboardColNum;
			callback(Vec2(col, row), checkerboard_[i]);
		}
	}
}

// 添加动作
void GameLogic::add_action(ActionType type, ChessPieceType chess_type, const Vec2 &source, const Vec2 &target)
{
	Action action;
	action.type = type;
	action.source = source;
	action.target = target;
	action.chess_type = chess_type;
	action_queue_.push(action);

	if (action_callback_ != nullptr)
	{
		action_callback_();
	}
}

// 是否在棋盘
bool GameLogic::is_in_checkerboard(const Vec2 &pos) const
{
	return pos.x >= 0 && pos.y >= 0 && pos.x < kCheckerboardColNum && pos.y < kCheckerboardRowNum;
}

// 棋子是否有效
bool GameLogic::is_valid_chess_piece(const Vec2 &pos) const
{
	return is_in_checkerboard(pos) && checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] != ChessPieceType::NONE;
}

// 获取棋子类型
GameLogic::ChessPieceType GameLogic::get_chesspiece_type(const Vec2 &pos) const
{
	return is_valid_chess_piece(pos) ? checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] : GameLogic::NONE;
}

// 是否相邻
bool GameLogic::is_adjacent(const Vec2 &a, const Vec2 &b) const
{
	return is_in_checkerboard(a) && is_in_checkerboard(a) && (abs(b.x - a.x) + abs(b.y - a.y) == 1);
}

// 移动棋子
bool GameLogic::move_chess_piece(const Vec2 &source, const Vec2 &target)
{
	assert(lock_ == false);
	lock_ = true;

	if (source != target)
	{
		if (is_valid_chess_piece(source) && !is_valid_chess_piece(target) && is_adjacent(source, target))
		{
			// 交换数据
			std::swap(checkerboard_[source.y  * kCheckerboardRowNum + source.x], checkerboard_[target.y  * kCheckerboardRowNum + target.x]);

			// 新增动作
			add_action(ActionType::MOVED, checkerboard_[target.y  * kCheckerboardRowNum + target.x], source, target);

			// 检测吃子
			std::set<Vec2> killed_set = GameLogic::check_kill_chesspiece(checkerboard_, target);
			for (auto &pos : killed_set)
			{
				checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] = ChessPieceType::NONE;
				add_action(ActionType::KILLED, ChessPieceType::NONE, target, pos);
			}

			// 玩家待机
			ChessPieceType type = checkerboard_[target.y  * kCheckerboardRowNum + target.x];
			type = type == ChessPieceType::WHITE ? ChessPieceType::BLACK : ChessPieceType::WHITE;
			add_action(ActionType::STANDBY, type, Vec2(), Vec2());

			lock_ = false;
			return true;
		}
	}

	lock_ = false;
	return false;
}

// 获取横向相连的棋子
std::vector<GameLogic::Vec2> GameLogic::get_chesspieces_with_horizontal(const ChessArray &checkerboard, const Vec2 &pos)
{
	int continuous = 0;
	std::vector<Vec2> ret;
	int last_type = checkerboard[pos.y * kCheckerboardColNum];
	for (int i = 0; i < kCheckerboardColNum; ++i)
	{
		int idx = pos.y * kCheckerboardColNum + i;
		if (checkerboard[idx] != ChessPieceType::NONE)
		{
			++continuous;
			ret.push_back(Vec2(i, pos.y));
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
std::vector<GameLogic::Vec2> GameLogic::get_chesspieces_with_vertical(const ChessArray &checkerboard, const Vec2 &pos)
{
	int continuous = 0;
	std::vector<Vec2> ret;
	int last_type = checkerboard[pos.y * kCheckerboardColNum];
	for (int i = 0; i < kCheckerboardRowNum; ++i)
	{
		int idx = i * kCheckerboardColNum + pos.x;
		if (checkerboard[idx] != ChessPieceType::NONE)
		{
			++continuous;
			ret.push_back(Vec2(pos.x, i));
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
std::set<GameLogic::Vec2> GameLogic::get_killed_chesspiece(const ChessArray &checkerboard, ChessPieceType key, const std::vector<Vec2> &chesspieces)
{
	std::set<Vec2> ret;
	if (chesspieces.size() == 3)
	{
		size_t sum = std::accumulate(chesspieces.begin(), chesspieces.end(), 0, [=](size_t sum, const Vec2 &item)->size_t
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
std::set<GameLogic::Vec2> GameLogic::check_kill_chesspiece(const ChessArray &checkerboard, const Vec2 &pos)
{
	auto key = checkerboard[pos.y * kCheckerboardColNum + pos.x];
	auto v_array = GameLogic::get_chesspieces_with_vertical(checkerboard, pos);
	auto h_array = GameLogic::get_chesspieces_with_horizontal(checkerboard, pos);
	auto horizontal_set = GameLogic::get_killed_chesspiece(checkerboard, key, h_array);
	auto vertical_set = GameLogic::get_killed_chesspiece(checkerboard, key, v_array);
	horizontal_set.insert(vertical_set.begin(), vertical_set.end());
	return horizontal_set;
}
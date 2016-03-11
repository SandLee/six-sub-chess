#include "SimpleRobot.h"

#include <ctime>
#include <random>
#include <cassert>
#include <algorithm>


SimpleRobot::SimpleRobot(GameLogic *logic)
	: logic_(logic)
	, action_read_pos_(0)
	, chess_type_(GameLogic::ChessPieceType::NONE)
{
	assert(logic_ != nullptr);
	logic_->add_action_update_callback(std::bind(&SimpleRobot::update_action, this));
}

SimpleRobot::~SimpleRobot()
{

}

// 更新动作
void SimpleRobot::update_action()
{
	run_action();
}

// 获取所有可行的移动路径
std::vector<GameLogic::MoveTrack> SimpleRobot::get_all_movetrack(const GameLogic::ChessArray &checkerboard, GameLogic::ChessPieceType type) const
{
	size_t idx = 0;
	std::vector<GameLogic::MoveTrack> track_array;
	while (idx != checkerboard.size())
	{
		if (checkerboard[idx] == type)
		{
			// 判断上下左右是否可移动
			int row = idx / GameLogic::kCheckerboardColNum;
			int col = idx % GameLogic::kCheckerboardColNum;
			for (int i = -1; i <= 1; i += 2)
			{
				GameLogic::Vec2 v1(col + i, row);
				GameLogic::Vec2 v2(col, row + i);
				if (is_in_checkerboard(v1) && !is_valid_chess_piece(checkerboard, v1))
				{
					GameLogic::MoveTrack track = { GameLogic::Vec2(col, row), v1 };
					track_array.push_back(track);
				}
				if (is_in_checkerboard(v2) && !is_valid_chess_piece(checkerboard, v2))
				{
					GameLogic::MoveTrack track = { GameLogic::Vec2(col, row), v2 };
					track_array.push_back(track);
				}
			}
		}
		++idx;
	}
	return track_array;
}

// 获取可杀死敌方棋子的移动路径
std::vector<GameLogic::MoveTrack> SimpleRobot::get_can_kill_chess_movetrack(std::vector<GameLogic::MoveTrack> &track_array) const
{
	std::vector<GameLogic::MoveTrack> kill_chess_array;
	GameLogic::ChessArray checkerboard = logic_->get_checkerboard();
	for (size_t i = 0; i < track_array.size(); ++i)
	{
		// 模拟出棋
		GameLogic::Vec2 &source =  track_array[i].source;
		GameLogic::Vec2 &target = track_array[i].target;
		std::swap(checkerboard[source.y * GameLogic::kCheckerboardColNum + source.x], checkerboard[target.y * GameLogic::kCheckerboardColNum + target.x]);

		std::set<GameLogic::Vec2> kill_set =  GameLogic::check_kill_chesspiece(checkerboard, target);
		if (!kill_set.empty())
		{
			kill_chess_array.push_back(track_array[i]);
		}

		// 恢复棋盘
		std::swap(checkerboard[source.y * GameLogic::kCheckerboardColNum + source.x], checkerboard[target.y * GameLogic::kCheckerboardColNum + target.x]);
	}

	return kill_chess_array;
}

// 获取可躲避被杀棋的移动路径
std::vector<GameLogic::MoveTrack> SimpleRobot::get_can_avoid_chess_movetrack(std::vector<GameLogic::MoveTrack> &track_array) const
{
	std::vector<GameLogic::MoveTrack> avoid_chess_array = track_array;
	GameLogic::ChessArray checkerboard = logic_->get_checkerboard();
	auto other_chesspiece_type = get_chesspiece_type() == GameLogic::ChessPieceType::WHITE ? GameLogic::ChessPieceType::BLACK : GameLogic::ChessPieceType::WHITE;
	for (size_t i = 0; i < track_array.size(); ++i)
	{
		// 模拟出棋
		GameLogic::Vec2 &source = track_array[i].source;
		GameLogic::Vec2 &target = track_array[i].target;
		std::swap(checkerboard[source.y * GameLogic::kCheckerboardColNum + source.x], checkerboard[target.y * GameLogic::kCheckerboardColNum + target.x]);

		// 模拟对方出棋
		{
			std::vector<GameLogic::MoveTrack> other_track_array = get_all_movetrack(checkerboard, other_chesspiece_type);
			for (size_t j = 0; j < other_track_array.size(); ++j)
			{
				// 模拟出棋
				GameLogic::Vec2 &other_source = other_track_array[j].source;
				GameLogic::Vec2 &other_target = other_track_array[j].target;
				std::swap(checkerboard[other_source.y * GameLogic::kCheckerboardColNum + other_source.x], checkerboard[other_target.y * GameLogic::kCheckerboardColNum + other_target.x]);

				if (!GameLogic::check_kill_chesspiece(checkerboard, other_target).empty())
				{
					auto itr = std::find(avoid_chess_array.begin(), avoid_chess_array.end(), track_array[i]);
					if (itr != avoid_chess_array.end())
					{
						avoid_chess_array.erase(itr);
					}
				}

				// 恢复棋盘
				std::swap(checkerboard[other_source.y * GameLogic::kCheckerboardColNum + other_source.x], checkerboard[other_target.y * GameLogic::kCheckerboardColNum + other_target.x]);
			}
		}

		// 恢复棋盘
		std::swap(checkerboard[source.y * GameLogic::kCheckerboardColNum + source.x], checkerboard[target.y * GameLogic::kCheckerboardColNum + target.x]);
	}

	return avoid_chess_array;
}

// 执行动作
void SimpleRobot::run_action()
{
	GameLogic::Action action = logic_->get_action_from_queue(action_read_pos_);
	if (action.type != GameLogic::ActionType::NONE)
	{
		if (action.type == GameLogic::ActionType::STANDBY &&
			action.chess_type != get_chesspiece_type())
		{
			// 获取所有可行的移动轨迹
			const GameLogic::ChessArray &checkerboard = logic_->get_checkerboard();
			std::vector<GameLogic::MoveTrack> track_array = get_all_movetrack(checkerboard, get_chesspiece_type());

			if (!track_array.empty())
			{
				// 获取可杀死敌方棋子的移动轨迹
				std::vector<GameLogic::MoveTrack> kill_chess_array = get_can_kill_chess_movetrack(track_array);

				// 优先杀死对方棋子，其次躲避对方
				if (!kill_chess_array.empty())
				{
					logic_->move_chess_piece(kill_chess_array.front().source, kill_chess_array.front().target);
				}
				else
				{
					std::vector<GameLogic::MoveTrack> avoid_chess_array = get_can_avoid_chess_movetrack(track_array);
					if (avoid_chess_array.empty())
					{
						std::default_random_engine generator(time(nullptr));
						std::uniform_int_distribution<int> dis(0, track_array.size() - 1);
						const GameLogic::MoveTrack &track = track_array[dis(generator)];
						logic_->move_chess_piece(track.source, track.target);
					}
					else
					{
						std::default_random_engine generator(time(nullptr));
						std::uniform_int_distribution<int> dis(0, avoid_chess_array.size() - 1);
						const GameLogic::MoveTrack &track = avoid_chess_array[dis(generator)];
						logic_->move_chess_piece(track.source, track.target);
					}
				}
			}
		}

		action_finished();
	}
}

// 完成动作
void SimpleRobot::action_finished()
{
	++action_read_pos_;
	update_action();
}

// 重置
void SimpleRobot::reset(GameLogic::ChessPieceType type)
{
	chess_type_ = type;
	action_read_pos_ = 0;
}

// 获取棋子类型
GameLogic::ChessPieceType SimpleRobot::get_chesspiece_type() const
{
	return chess_type_;
}
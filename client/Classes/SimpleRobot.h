#ifndef __SIMPLEROBOT_H__
#define __SIMPLEROBOT_H__

#include <memory>
#include "GameLogic.h"

class SimpleRobot
{
public:
	SimpleRobot(GameLogic *logic);
	~SimpleRobot();

public:
	/**
	 * 更新动作
	 */
	void update_action();

	/**
	 * 执行动作
	 */
	void run_action();

	/**
	 * 完成动作
	 */
	void action_finished();

	/**
	 * 重置
	 */
	void reset(GameLogic::ChessPieceType type);

	/**
	 * 获取棋子类型
	 */
	GameLogic::ChessPieceType get_chesspiece_type() const;

public:
	/**
	 * 获取可杀死敌方棋子的移动路径
	 */
	std::vector<GameLogic::MoveTrack> get_can_kill_chess_movetrack(std::vector<GameLogic::MoveTrack> &track_array) const;

	/**
	 * 获取可躲避被杀棋的移动路径
	 */
	std::vector<GameLogic::MoveTrack> get_can_avoid_chess_movetrack(std::vector<GameLogic::MoveTrack> &track_array) const;

	/**
	 * 获取所有可行的移动路径
	 */
	std::vector<GameLogic::MoveTrack> get_all_movetrack(const GameLogic::ChessArray &checkerboard, GameLogic::ChessPieceType type) const;

protected:
	SimpleRobot(const SimpleRobot &) = delete;
	SimpleRobot& operator= (const SimpleRobot &) = delete;

private:
	// 是否在棋盘
	bool is_in_checkerboard(const GameLogic::Vec2 &pos) const
	{
		return pos.x >= 0 && pos.y >= 0 && pos.x < GameLogic::kCheckerboardColNum && pos.y < GameLogic::kCheckerboardRowNum;
	}

	// 棋子是否有效
	bool is_valid_chess_piece(const GameLogic::ChessArray &checkerboard, const GameLogic::Vec2 &pos) const
	{
		return is_in_checkerboard(pos) && checkerboard[pos.y  * GameLogic::kCheckerboardRowNum + pos.x] != GameLogic::ChessPieceType::NONE;
	}

private:
	GameLogic*					logic_;
	GameLogic::ChessPieceType	chess_type_;
	int							action_read_pos_;
};

#endif
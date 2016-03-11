#ifndef __SIMPLEROBOT_H__
#define __SIMPLEROBOT_H__

#include <memory>
#include "SingleLogic.h"

class SimpleRobot
{
public:
	SimpleRobot(SingleLogic *logic);
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
	void reset(FChessPieceType type);

	/**
	 * 获取棋子类型
	 */
	FChessPieceType get_chesspiece_type() const;

public:
	/**
	 * 获取可杀死敌方棋子的移动路径
	 */
	std::vector<FMoveTrack> get_can_kill_chess_movetrack(std::vector<FMoveTrack> &track_array) const;

	/**
	 * 获取可躲避被杀棋的移动路径
	 */
	std::vector<FMoveTrack> get_can_avoid_chess_movetrack(std::vector<FMoveTrack> &track_array) const;

	/**
	 * 获取所有可行的移动路径
	 */
	std::vector<FMoveTrack> get_all_movetrack(const FChessArray &checkerboard, FChessPieceType type) const;

protected:
	SimpleRobot(const SimpleRobot &) = delete;
	SimpleRobot& operator= (const SimpleRobot &) = delete;

private:
	// 是否在棋盘
	bool is_in_checkerboard(const FVec2 &pos) const
	{
		return pos.x >= 0 && pos.y >= 0 && pos.x < kCheckerboardColNum && pos.y < kCheckerboardRowNum;
	}

	// 棋子是否有效
	bool is_valid_chesspiece(const FChessArray &checkerboard, const FVec2 &pos) const
	{
		return is_in_checkerboard(pos) && checkerboard[pos.y  * kCheckerboardRowNum + pos.x] != FChessPieceType::NONE;
	}

private:
	SingleLogic*	logic_;
	FChessPieceType	chess_type_;
	int				action_read_pos_;
};

#endif
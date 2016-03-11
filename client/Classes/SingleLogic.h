#ifndef __SINGLELOGIC_H__
#define __SINGLELOGIC_H__

#include "LogicBase.h"

class SingleLogic : public LogicBase
{
public:
	/**
	 * 准备开始
	 */
	virtual void ready() override;

	/**
	 * 获取上方玩家棋子类型
	 */
	virtual FChessPieceType get_upperplayer_chesspiece_type() const override;

	/**
	* 获取下方玩家棋子类型
	*/
	virtual FChessPieceType get_belowplayer_chesspiece_type() const override;

	/**
	 * 移动棋子
	 */
	virtual void move_chesspiece(const FVec2 &source, const FVec2 &target) override;

private:
	FChessPieceType upperplayer_;
};

#endif
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
	 * 移动棋子
	 */
	virtual void move_chess_piece(const FVec2 &source, const FVec2 &target) override;
};

#endif
#ifndef __CLIENTLOGIC_H__
#define __CLIENTLOGIC_H__

#include "LogicBase.h"
#include "CocosNet/cocos-net.h"

class ClientLogic : public LogicBase, public cocos2d::NetDelegate
{
public:
	// 读取数据
	virtual void onMessageReceived(cocos2d::Buffer& oBuffer) override;

	// 连接成功
	virtual void onConnected() override;

	// 连接失败
	virtual void onConnectTimeout() override;

	// 断开连接
	virtual void onDisconnected() override;

	// 连接异常
	virtual void onExceptionCaught(cocos2d::SocketStatus eStatus) override;

public:
	// 准备开始
	virtual void ready() override;
	
	// 获取上方玩家棋子类型
	virtual FChessPieceType get_upperplayer_chesspiece_type() const override;

	// 获取下方玩家棋子类型
	virtual FChessPieceType get_belowplayer_chesspiece_type() const override;

	// 移动棋子
	virtual void move_chesspiece(const FVec2 &source, const FVec2 &target) override;
};

#endif
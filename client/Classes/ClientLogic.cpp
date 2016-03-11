#include "ClientLogic.h"
using namespace cocos2d;


// 读取数据
void ClientLogic::onMessageReceived(Buffer &oBuffer)
{
	// 开始游戏
	// 移动棋子
	// 游戏结束
}

// 连接成功
void ClientLogic::onConnected()
{
	// 匹配房间
}

// 连接失败
void ClientLogic::onConnectTimeout()
{

}

// 断开连接
void ClientLogic::onDisconnected()
{

}

// 连接异常
void ClientLogic::onExceptionCaught(SocketStatus eStatus)
{

}

// 准备开始
void ClientLogic::ready()
{
	// 连接服务器
	setInetAddress(InetAddress("192.168.1.139", 4235));
	connect();
}

// 获取上方玩家棋子类型
FChessPieceType ClientLogic::getUpperplayerChesspieceType() const
{
	return FChessPieceType();
}

// 获取下方玩家棋子类型
FChessPieceType ClientLogic::getBelowplayerChesspieceType() const
{
	return FChessPieceType();
}

// 移动棋子
void ClientLogic::moveChesspiece(const FVec2 &source, const FVec2 &target)
{
	// 移动棋子
}
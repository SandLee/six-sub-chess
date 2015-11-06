#ifndef __LOGICHANDLE_H__
#define __LOGICHANDLE_H__


#include <queue>
#include <array>
#include <functional>
#include "cocos2d.h"
#include "Singleton.h"


class LogicHandle : public Singleton<LogicHandle>
{
	SINGLETON(LogicHandle);

public:
	enum class EventType
	{
		None,
		Moved,				// 移动了
		Killed,				// 吃掉了
	};

	struct EventDetails
	{
		EventType type;
		cocos2d::Vec2 source;
		cocos2d::Vec2 target;
	};

	typedef std::function<void()> EventUpate;

	static const int kCheckerboardRowNum = 4;		// 棋盘行数
	static const int kCheckerboardColNum = 4;		// 棋盘列数


public:
	/**
	 * 初始化棋盘
	 */
	void init_checkerboard();

	/**
	 * 添加事件更新通知
	 */
	void add_event_update_notice(EventUpate &&func);

	/**
	 * 取出事件信息
	 */
	EventDetails take_event_info();

	/**
	 * 是否在棋盘
	 */
	bool is_in_checkerboard(const cocos2d::Vec2 &pos) const;

	/**
	 * 棋子是否有效
	 */
	bool is_valid_chess_piece(const cocos2d::Vec2 &pos) const;

	/**
	 * 是否相邻
	 */
	bool is_adjacent(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b) const;

	/**
	 * 移动棋子
	 */
	bool move_chess_piece(const cocos2d::Vec2 &source, const cocos2d::Vec2 &target);

	/**
	 * 定时器更新
	 */
	void update(float dt);

private:
	unsigned int												hander_num_;
	EventUpate													even_update_;
	std::queue<EventDetails>									event_queue_;
	std::array<int, kCheckerboardRowNum * kCheckerboardColNum>	checkerboard_;
};

#endif
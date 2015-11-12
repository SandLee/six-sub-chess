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
	enum ChessPieceType
	{
		None,
		WHITE = 1,		// 白棋
		BLACK = 2,		// 黑棋
	};

	enum class EventType
	{
		NONE,
		MOVED,				// 移动了
		KILLED,				// 吃掉了
	};

	struct EventDetails
	{
		int chesspiece;
		EventType type;
		cocos2d::Vec2 source;
		cocos2d::Vec2 target;
	};

	static const int kCheckerboardRowNum = 4;		// 棋盘行数
	static const int kCheckerboardColNum = 4;		// 棋盘列数


public:
	/**
	 * 初始化棋盘
	 */
	void init_checkerboard();

	/**
	 * 浏览棋盘
	 */
	void visit_checkerboard(const std::function<void(const cocos2d::Vec2 &, int value)> &func);

	/**
	 * 添加事件更新通知
	 */
	void add_event_update_notice(std::function<void()> &&func);

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
	/**
	 * 获取横向相连的棋子
	 */
	std::vector<cocos2d::Vec2> get_chesspieces_with_horizontal(const cocos2d::Vec2 &pos) const;

	/**
	 * 获取纵向相连的棋子
	 */
	std::vector<cocos2d::Vec2> get_chesspieces_with_vertical(const cocos2d::Vec2 &pos) const;

	/**
	 * 获取可杀死的棋子
	 * @param std::vector<cocos2d::Vec2> 相连的棋子列表
	 * @param std::set<cocos2d::Vec2> 可杀死的棋子
	 */
	void get_killed_chesspiece(ChessPieceType key, const std::vector<cocos2d::Vec2> &chesspieces, std::set<cocos2d::Vec2> &ret) const;

	/**
	 * 检查杀死棋子
	 */
	std::set<cocos2d::Vec2> check_kill_chesspiece(const cocos2d::Vec2 &pos) const;

private:
	unsigned int															hander_num_;
	std::function<void()>													even_update_;
	std::queue<EventDetails>												event_queue_;
	std::array<ChessPieceType, kCheckerboardRowNum * kCheckerboardColNum>	checkerboard_;
};

#endif
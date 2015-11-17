#ifndef __GAMELOGIC_H__
#define __GAMELOGIC_H__

#include <set>
#include <array>
#include <queue>
#include <numeric>
#include <functional>

class GameLogic
{
public:
	/**
	 * 棋子类型枚举
	 */
	enum ChessPieceType
	{
		NONE,
		WHITE = 1,									// 白棋
		BLACK = 2,									// 黑棋
	};

	/**
	 * 动作类型枚举
	 */
	enum class ActionType
	{
		NONE,
		MOVED,										// 移动
		KILLED,										// 杀棋
		STANDBY,									// 待机
	};

	/**
	 * 二维坐标
	 */
	struct Vec2
	{
		int x;
		int y;

		Vec2() : x(0), y(0) {}
		Vec2(int _x, int _y) : x(_x), y(_y) {}

		bool operator!= (const Vec2 &that) const
		{
			return x != that.x || y != that.y;
		}

		bool operator== (const Vec2 &that) const
		{
			return x == that.x && y == that.y;
		}

		bool operator< (const Vec2 &that) const
		{
			return y < that.y ? true : y == that.y ? x < that.x : false;
		}
	};

	struct MoveTrack
	{
		GameLogic::Vec2 source;
		GameLogic::Vec2 target;

		bool operator== (const MoveTrack &that) const
		{
			return source == that.source && target == that.target;
		}
	};

	/**
	 * 动作信息
	 */
	struct Action
	{
		ActionType		type;						// 动作类型
		ChessPieceType	chess_type;					// 棋子类型
		Vec2			source;						// 来源位置
		Vec2			target;						// 目标位置
	};

	static const int kCheckerboardRowNum = 4;		// 棋盘行数
	static const int kCheckerboardColNum = 4;		// 棋盘列数

	typedef std::array<ChessPieceType, kCheckerboardRowNum * kCheckerboardColNum> ChessArray;

public:
	GameLogic(const ChessArray &checkerboard);

	~GameLogic();

	/**
	 * 清理
	 */
	void clear();

	/**
	 * 获取动作信息
	 */
	Action get_action_from_queue(size_t index);

	/**
	 * 添加动作更新回调
	 */
	void add_action_update_callback(std::function<void()> &&callback);

	/**
	* 获取棋盘数据
	*/
	const ChessArray& get_checkerboard() const;

	/**
	* 浏览棋盘
	*/
	void visit_checkerboard(const std::function<void(const Vec2&, ChessPieceType type)> &callback);

	/**
	 * 是否在棋盘
	 */
	bool is_in_checkerboard(const Vec2 &pos) const;

	/**
	 * 棋子是否有效
	 */
	bool is_valid_chess_piece(const Vec2 &pos) const;

	/**
	 * 获取棋子类型
	 */
	GameLogic::ChessPieceType get_chesspiece_type(const Vec2 &pos) const;

	/**
	 * 是否相邻
	 */
	bool is_adjacent(const Vec2 &a, const Vec2 &b) const;

	/**
	 * 移动棋子
	 */
	void move_chess_piece(const Vec2 &source, const Vec2 &target);

	/**
	 * 更新
	 */
	void update(float dt);

public:
	/**
	 * 获取横向相连的棋子
	 * @param ChessArray 棋牌信息
	 * @param Vec2 移动过的棋子的位置
	 */
	static std::vector<Vec2> get_chesspieces_with_horizontal(const ChessArray &checkerboard, const Vec2 &pos);

	/**
	 * 获取纵向相连的棋子
	 * @param ChessArray 棋牌信息
	 * @param Vec2 移动过的棋子的位置
	 */
	static std::vector<Vec2> get_chesspieces_with_vertical(const ChessArray &checkerboard, const Vec2 &pos);

	/**
	 * 获取可杀死的棋子
	 * @param ChessArray 棋牌信息
	 * @param ChessPieceType 移动过的棋子的类型
	 * @param std::vector<Vec2> 相连的棋子列表
	 * @return std::set<Vec2> 可吃掉的棋子
	 */
	static std::set<Vec2> get_killed_chesspiece(const ChessArray &checkerboard, ChessPieceType key, const std::vector<Vec2> &chesspieces);

	/**
	 * 检查可吃掉的棋子
	 * @param ChessArray 棋牌信息
	 * @param Vec2 移动过的棋子的位置
	 */
	static std::set<Vec2> check_kill_chesspiece(const ChessArray &checkerboard, const Vec2 &pos);

protected:
	GameLogic(const GameLogic&) = delete;
	GameLogic& operator= (const GameLogic&) = delete;

private:
	/**
	 * 添加动作
	 */
	void add_action(ActionType type, ChessPieceType chess_type, const Vec2 &source, const Vec2 &target);

private:
	ChessPieceType						standby_type_;
	std::queue<MoveTrack>				move_queue_;
	std::vector<Action>					action_queue_;
	ChessArray							checkerboard_;
	std::vector<std::function<void()>>	action_callback_list_;
};

#endif
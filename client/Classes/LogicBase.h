#ifndef __LOGICBASE_H__
#define __LOGICBASE_H__

#include <set>
#include <array>
#include <queue>
#include <numeric>
#include <functional>

/**
 * 棋子类型枚举
 */
enum FChessPieceType
{
	NONE,
	WHITE = 1,									// 白棋
	BLACK = 2,									// 黑棋
};

/**
 * 动作类型枚举
 */
enum class FActionType
{
	NONE,
	BEREADY,									// 准备好了
	START,										// 开始
	MOVED,										// 移动
	KILLED,										// 杀棋
	STANDBY,									// 待机
	GAMEOVER,									// 游戏结束
};

/**
 * 二维坐标
 */
struct FVec2
{
	int x;
	int y;

	FVec2() : x(0), y(0) {}
	FVec2(int _x, int _y) : x(_x), y(_y) {}

	static FVec2 invalid()
	{
		return FVec2(-1, -1);
	}

	bool operator!= (const FVec2 &that) const
	{
		return x != that.x || y != that.y;
	}

	bool operator== (const FVec2 &that) const
	{
		return x == that.x && y == that.y;
	}

	bool operator< (const FVec2 &that) const
	{
		return y < that.y ? true : y == that.y ? x < that.x : false;
	}
};

/**
 * 移动轨迹
 */
struct FMoveTrack
{
	FVec2 source;
	FVec2 target;

	bool operator== (const FMoveTrack &that) const
	{
		return source == that.source && target == that.target;
	}
};

/**
 * 动作信息
 */
struct FAction
{
	FActionType		type;						// 动作类型
	FChessPieceType	chess_type;					// 棋子类型
	FVec2			source;						// 来源位置
	FVec2			target;						// 目标位置
};

static const int kCheckerboardRowNum = 4;		// 棋盘行数
static const int kCheckerboardColNum = 4;		// 棋盘列数

typedef std::array<FChessPieceType, kCheckerboardRowNum * kCheckerboardColNum> FChessArray;

class LogicBase
{
public:
	LogicBase();
	virtual ~LogicBase();

public:
	/**
	 * 准备开始
	 */
	virtual void ready() = 0;

	/**
	 * 获取上方玩家棋子类型
	 */
	virtual FChessPieceType getUpperplayerChesspieceType() const = 0;

	/**
	* 获取下方玩家棋子类型
	*/
	virtual FChessPieceType getBelowplayerChesspieceType() const = 0;

	/**
	 * 移动棋子
	 */
	virtual void moveChesspiece(const FVec2 &source, const FVec2 &target) = 0;

public:
	/**
	 * 获取动作数量
	 */
	size_t getActionNum() const;

	/**
	 * 获取动作信息
	 */
	FAction getActionFromQueue(size_t index);

	/**
	 * 添加动作更新回调
	 */
	void addActionUpdateCallback(const std::function<void()> &callback);

	/**
	 * 获取棋盘数据
	 */
	const FChessArray& getCheckerboard() const;

	/**
	 * 浏览棋盘
	 */
	void visitCheckerboard(const std::function<void(const FVec2&, FChessPieceType type)> &callback);

	/**
	 * 是否在棋盘
	 */
	bool isInCheckerboard(const FVec2 &pos) const;

	/**
	 * 棋子是否有效
	 */
	bool isValidChesspiece(const FVec2 &pos) const;

	/**
	 * 获取棋子类型
	 */
	FChessPieceType getChesspieceType(const FVec2 &pos) const;

	/**
	 * 获取待机棋子类型
	 */
	FChessPieceType getStandbyChesspieceType() const;

	/**
	 * 是否相邻
	 */
	bool isAdjacent(const FVec2 &a, const FVec2 &b) const;

	/**
	 * 更新
	 */
	void update(float dt);

protected:
	LogicBase(const LogicBase &) = delete;
	LogicBase& operator= (const LogicBase &) = delete;

protected:
	/**
	 * 重置
	 */
	void reset();

	/**
	 * 设置棋盘
	 */
	void setCheckerboard(const FChessArray &checkerboard);

	/**
	 * 添加移动轨迹
	 */
	void addMovetrack(const FMoveTrack &track);

	/**
	 * 添加动作
	 */
	void addAction(FActionType type, FChessPieceType chess_type, const FVec2 &source, const FVec2 &target);

private:
	/**
	 * 获取所有可行的移动路径
	 */
	std::vector<FMoveTrack> getAllMovetrack(FChessPieceType type) const;

private:
	FChessPieceType							standby_chess_type_;
	std::queue<FMoveTrack>					move_queue_;
	std::vector<FAction>					action_queue_;
	FChessArray								checkerboard_;
	std::vector< std::function<void()> >	action_callback_list_;
};

namespace helper
{
	/**
	 * 获取横向相连的棋子
	 * @param FChessArray 棋牌信息
	 * @param FVec2 移动过的棋子的位置
	 */
	std::vector<FVec2> GetChesspiecesWithHorizontal(const FChessArray &checkerboard, const FVec2 &pos);

	/**
	 * 获取纵向相连的棋子
	 * @param FChessArray 棋牌信息
	 * @param FVec2 移动过的棋子的位置
	 */
	std::vector<FVec2> GetChesspiecesWithVertical(const FChessArray &checkerboard, const FVec2 &pos);

	/**
	 * 获取可杀死的棋子
	 * @param FChessArray 棋牌信息
	 * @param FChessPieceType 移动过的棋子的类型
	 * @param std::vector<FVec2> 相连的棋子列表
	 * @return std::set<FVec2> 可吃掉的棋子
	 */
	std::set<FVec2> GetKilledChesspiece(const FChessArray &checkerboard, FChessPieceType key, const std::vector<FVec2> &chesspieces);

	/**
	 * 检查可吃掉的棋子
	 * @param FChessArray 棋牌信息
	 * @param FVec2 移动过的棋子的位置
	 */
	std::set<FVec2> CheckKillChesspiece(const FChessArray &checkerboard, const FVec2 &pos);
}

#endif
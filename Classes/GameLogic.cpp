#include "GameLogic.h"

#include <numeric>
#include "json/document.h"
using namespace cocos2d;


GameLogic::GameLogic()
	: hander_num_(0)
{
	init_checkerboard();

	// 开启定时器
	Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
}

GameLogic::~GameLogic()
{
	// 关闭定时器
	Director::getInstance()->getScheduler()->unscheduleUpdate(this);
}

// 初始化棋盘
void GameLogic::init_checkerboard()
{
	Data data = FileUtils::getInstance()->getDataFromFile("config/init.json");

	// 解析json
	rapidjson::Document doc;
	doc.Parse<0>(std::string((const char *)data.getBytes(), data.getSize()).c_str());
	if (doc.HasParseError() || !doc.IsArray())
	{
		CCAssert(false, "Json parse error!");
	}

	if (doc.Size() != kCheckerboardRowNum * kCheckerboardColNum)
	{
		CCAssert(false, "Array size error!");
	}

	for (size_t i = 0; i < doc.Size(); ++i)
	{
		int row = i / kCheckerboardColNum;
		int col = i % kCheckerboardColNum;
		checkerboard_[row * kCheckerboardRowNum + col] = static_cast<ChessPieceType>(doc[doc.Size() - i - 1].GetInt());
	}

	hander_num_ = 0;
}

// 浏览棋盘
void GameLogic::visit_checkerboard(const std::function<void(const cocos2d::Vec2 &, int value)> &func)
{
	if (func != nullptr)
	{
		for (size_t i = 0; i < checkerboard_.size(); ++i)
		{
			int row = i / kCheckerboardColNum;
			int col = i % kCheckerboardColNum;
			func(Vec2(col, row), checkerboard_[i]);
		}
	}
}

// 添加事件更新通知
void GameLogic::add_event_update_notice(std::function<void()> &&func)
{
	even_update_ = func;
}

// 取出事件信息
GameLogic::EventDetails GameLogic::take_event_info()
{
	EventDetails ret;
	ret.chess_type = ChessPieceType::NONE;
	ret.type = EventType::NONE;
	if (!event_queue_.empty())
	{
		ret = event_queue_.front();
		event_queue_.pop();
	}
	return ret;
}

// 是否在棋盘
bool GameLogic::is_in_checkerboard(const Vec2 &pos) const
{
	return pos.x >= 0 && pos.y >= 0 && pos.x < kCheckerboardColNum && pos.y < kCheckerboardRowNum;
}

// 棋子是否有效
bool GameLogic::is_valid_chess_piece(const Vec2 &pos) const
{
	return is_in_checkerboard(pos) && checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] != ChessPieceType::NONE;
}

// 获取棋子类型
GameLogic::ChessPieceType GameLogic::get_chesspiece_type(const Vec2 &pos) const
{
	if (!is_valid_chess_piece(pos))
	{
		return GameLogic::NONE;
	}
	return checkerboard_[pos.y  * kCheckerboardRowNum + pos.x];
}

// 是否相邻
bool GameLogic::is_adjacent(const Vec2 &a, const Vec2 &b) const
{
	return is_in_checkerboard(a) && is_in_checkerboard(a) && (abs(b.x - a.x) + abs(b.y - a.y) == 1);
}

// 移动棋子
bool GameLogic::move_chess_piece(const Vec2 &source, const Vec2 &target)
{
	if (source != target)
	{
		if (is_valid_chess_piece(source) && !is_valid_chess_piece(target) && is_adjacent(source, target))
		{
			// 交换数据
			std::swap(checkerboard_[source.y  * kCheckerboardRowNum + source.x], checkerboard_[target.y  * kCheckerboardRowNum + target.x]);

			// 新增事件
			++hander_num_;
			EventDetails event;
			event.type = EventType::MOVED;
			event.source = source;
			event.target = target;
			event.chess_type = checkerboard_[target.y  * kCheckerboardRowNum + target.x];
			event_queue_.push(event);

			// 检测吃子
			std::set<Vec2> killed_set = GameLogic::check_kill_chesspiece(checkerboard_, target);
			for (auto &pos : killed_set)
			{
				checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] = ChessPieceType::NONE;
				event.type = EventType::KILLED;
				event.source = target;
				event.target = pos;
				event_queue_.push(event);
			}

			// 通知出棋
			ChessPieceType chess_type = event.chess_type == ChessPieceType::WHITE ? ChessPieceType::BLACK : ChessPieceType::WHITE;
			for (size_t i = 0; i < checkerboard_.size(); ++i)
			{
				if (checkerboard_[i] == chess_type)
				{
					// 优先计算能否吃子
					

					// 其次计算能否被吃
				}
			}
		}
	}
	return false;
}

// 定时器更新
void GameLogic::update(float dt)
{
	while (hander_num_ > 0)
	{
		--hander_num_;
		if (even_update_ != nullptr)
		{
			even_update_();
		}
	}
}

// 获取横向相连的棋子
std::vector<Vec2> GameLogic::get_chesspieces_with_horizontal(const ChessArray &checkerboard, const Vec2 &pos)
{
	int continuous = 0;
	std::vector<Vec2> ret;
	int last_type = checkerboard[pos.y * kCheckerboardColNum];
	for (int i = 0; i < kCheckerboardColNum; ++i)
	{
		int idx = pos.y * kCheckerboardColNum + i;
		if (checkerboard[idx] != ChessPieceType::NONE)
		{
			++continuous;
			ret.push_back(Vec2(i, pos.y));
		}
		else
		{
			if (continuous == 3)
			{
				break;
			}
			else
			{
				continuous = 0;
				ret.clear();
			}
		}
	}

	if (ret.size() != 3)
	{
		ret.clear();
	}

	return ret;
}

// 获取纵向相连的棋子
std::vector<Vec2> GameLogic::get_chesspieces_with_vertical(const ChessArray &checkerboard, const cocos2d::Vec2 &pos)
{
	int continuous = 0;
	std::vector<Vec2> ret;
	int last_type = checkerboard[pos.y * kCheckerboardColNum];
	for (int i = 0; i < kCheckerboardRowNum; ++i)
	{
		int idx = i * kCheckerboardColNum + pos.x;
		if (checkerboard[idx] != ChessPieceType::NONE)
		{
			++continuous;
			ret.push_back(Vec2(pos.x, i));
		}
		else
		{
			if (continuous == 3)
			{
				break;
			}
			else
			{
				continuous = 0;
				ret.clear();
			}
		}
	}

	if (ret.size() != 3)
	{
		ret.clear();
	}

	return ret;
}

// 获取可吃掉的棋子
std::set<Vec2> GameLogic::get_killed_chesspiece(const ChessArray &checkerboard, ChessPieceType key, const std::vector<Vec2> &chesspieces)
{
	std::set<Vec2> ret;
	if (chesspieces.size() == 3)
	{
		size_t sum = std::accumulate(chesspieces.begin(), chesspieces.end(), 0, [=](size_t sum, const Vec2 &item)->size_t
		{
			if (key == checkerboard[item.y * kCheckerboardColNum + item.x])
			{
				return sum + 1;
			}
			return sum;
		});

		if (sum == 2)
		{
			for (size_t i = 0; i < chesspieces.size(); ++i)
			{
				int index = chesspieces[i].y * kCheckerboardColNum + chesspieces[i].x;
				if (i != 1 && checkerboard[index] != key)
				{
					ret.insert(chesspieces[i]);
				}
			}
		}
	}

	return ret;
}

// 检查杀死棋子
std::set<Vec2> GameLogic::check_kill_chesspiece(const ChessArray &checkerboard, const cocos2d::Vec2 &pos)
{
	auto key = checkerboard[pos.y * kCheckerboardColNum + pos.x];
	auto v_array = GameLogic::get_chesspieces_with_vertical(checkerboard, pos);
	auto h_array = GameLogic::get_chesspieces_with_horizontal(checkerboard, pos);
	auto horizontal_set = GameLogic::get_killed_chesspiece(checkerboard, key, h_array);
	auto vertical_set = GameLogic::get_killed_chesspiece(checkerboard, key, v_array);
	horizontal_set.insert(vertical_set.begin(), vertical_set.end());
	return horizontal_set;
}
#include "LogicHandle.h"

#include "json/document.h"
using namespace cocos2d;


LogicHandle::LogicHandle()
	: hander_num_(0)
{
	init_checkerboard();

	// 开启定时器
	Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
}

LogicHandle::~LogicHandle()
{
	// 关闭定时器
	Director::getInstance()->getScheduler()->unscheduleUpdate(this);
}

// 初始化棋盘
void LogicHandle::init_checkerboard()
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
		checkerboard_[row * kCheckerboardRowNum + col] = doc[doc.Size() - i - 1].GetInt();
	}

	hander_num_ = 0;
}

// 浏览棋盘
void LogicHandle::visit_checkerboard(const std::function<void(const cocos2d::Vec2 &, int value)> &func)
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
void LogicHandle::add_event_update_notice(std::function<void()> &&func)
{
	even_update_ = func;
}

// 取出事件信息
LogicHandle::EventDetails LogicHandle::take_event_info()
{
	EventDetails ret;
	ret.type = EventType::None;
	if (!event_queue_.empty())
	{
		ret = event_queue_.front();
		event_queue_.pop();
	}
	return ret;
}

// 是否在棋盘
bool LogicHandle::is_in_checkerboard(const cocos2d::Vec2 &pos) const
{
	return pos.x >= 0 && pos.y >= 0 && pos.x < kCheckerboardColNum && pos.y < kCheckerboardRowNum;
}

// 棋子是否有效
bool LogicHandle::is_valid_chess_piece(const cocos2d::Vec2 &pos) const
{
	return is_in_checkerboard(pos) && checkerboard_[pos.y  * kCheckerboardRowNum + pos.x] != 0;
}

// 是否相邻
bool LogicHandle::is_adjacent(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b) const
{
	return is_in_checkerboard(a) && is_in_checkerboard(a) && abs(b.x - a.x + b.y - a.y) == 1;
}

// 移动棋子
bool LogicHandle::move_chess_piece(const cocos2d::Vec2 &source, const cocos2d::Vec2 &target)
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
			event.type = EventType::Moved;
			event.source = source;
			event.target = target;
			event_queue_.push(event);
		}
	}
	return false;
}

// 定时器更新
void LogicHandle::update(float dt)
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
#include "Checkerboard.h"

#include <random>
#include "VisibleRect.h"
using namespace cocos2d;


const float kMinMoveDistance = 5.0f;
const Vec2 kInvalidCheckerboardPos(-1.0f, -1.0f);


// 七色
static std::vector<Color4B> g_seven_colors =
{
	Color4B(255, 0, 0, 255),
	Color4B(255, 165, 0, 255),
	Color4B(255, 255, 0, 255),
	Color4B(0, 128, 0, 255),
	Color4B(0, 255, 255, 255),
	Color4B(0, 0, 255, 255),
	Color4B(79, 47, 79, 255)
};

Checkerboard::Checkerboard()
	: real_move_(false)
	, action_lock_(false)
{
	for (size_t i = 0; i < chesspiece_sprite_.size(); ++i)
	{
		chesspiece_sprite_[i] = nullptr;
	}
}

Checkerboard::~Checkerboard()
{

}

// 获取棋子开始位置
Vec2 Checkerboard::get_chesspiece_start_pos() const
{
	Vec2 start_pos;
	start_pos.x = VisibleRect::center().x - (kChessPieceWidth * LogicHandle::kCheckerboardColNum + kInterval * (LogicHandle::kCheckerboardColNum - 1)) / 2;
	start_pos.y = VisibleRect::center().y - (kChessPieceHeight * LogicHandle::kCheckerboardRowNum + kInterval * (LogicHandle::kCheckerboardRowNum - 1)) / 2;
	return start_pos;
}

bool Checkerboard::init()
{
	if (!Layer::init())
	{
		return false;
	}

	// 初始化棋盘
	Vec2 start_pos = get_chesspiece_start_pos();
	for (size_t i = 0; i < color_floor_.size(); ++i)
	{
		int row = i / LogicHandle::kCheckerboardRowNum;
		int col = i % LogicHandle::kCheckerboardRowNum;
		auto layer = LayerColor::create();
		layer->setContentSize(Size(kChessPieceWidth, kChessPieceHeight));
		layer->setPosition(start_pos + Vec2(col * kChessPieceWidth + col * kInterval, row * kChessPieceHeight + row * kInterval));
		color_floor_[i] = layer;
		addChild(layer);
	}

	// 刷新棋盘
	refresh_checkerboard();

	// 注册棋盘事件
	LogicHandle::instance()->add_event_update_notice(std::bind(&Checkerboard::update_action, this));

	// 开启触摸
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);

	listener->onTouchBegan = CC_CALLBACK_2(Checkerboard::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(Checkerboard::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(Checkerboard::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(Checkerboard::onTouchCancelled, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

// 刷新棋盘
void Checkerboard::refresh_checkerboard()
{
	// 生成地板
	std::default_random_engine generator(time(nullptr));
	std::uniform_int_distribution<int> dis(0, g_seven_colors.size() - 1);
	const Color4B color = g_seven_colors[dis(generator)];

	for (size_t i = 0; i < color_floor_.size(); ++i)
	{
		int index = (i % LogicHandle::kCheckerboardRowNum) % 2;
		if (color_floor_[i] != nullptr)
		{
			color_floor_[i]->initWithColor(color, kChessPieceWidth, kChessPieceHeight);
		}
	}

	// 清理棋子精灵
	for (size_t i = 0; i < chesspiece_sprite_.size(); ++i)
	{
		if (chesspiece_sprite_[i] != nullptr)
		{
			free_sprite_.push_back(chesspiece_sprite_[i]);
			chesspiece_sprite_[i]->setVisible(false);
			chesspiece_sprite_[i]->stopAllActions();
			chesspiece_sprite_[i] = nullptr;
		}
	}

	// 绘制棋子精灵
	Vec2 start_pos = get_chesspiece_start_pos();
	LogicHandle::instance()->visit_checkerboard([&](const Vec2 &pos, int value)
	{
		if (value != 0)
		{
			Vec2 new_pos(start_pos + Vec2(pos.x * kChessPieceWidth + pos.x * kInterval + kChessPieceWidth / 2,
				pos.y * kChessPieceHeight + pos.y * kInterval + kChessPieceHeight / 2));

			Sprite *chess_piece = nullptr;
			if (free_sprite_.empty())
			{
				chess_piece  = Sprite::create(value == 1 ? "whiteplay.png" : "blackplay.png");
				addChild(chess_piece);
			}
			else
			{
				chess_piece = free_sprite_.back();
				free_sprite_.pop_back();
				Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(value == 1 ? "whiteplay.png" : "blackplay.png");
				chess_piece->setTexture(texture);
				chess_piece->setTextureRect(Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
				chess_piece->setVisible(true);
			}
			chess_piece->setPosition(new_pos);
			int index = pos.y * LogicHandle::kCheckerboardColNum + pos.x;
			assert(chesspiece_sprite_[index] == nullptr);
			chesspiece_sprite_[index] = chess_piece;
		}
	});
}

// 棋盘坐标转换到世界坐标
Vec2 Checkerboard::convert_to_world_space(const Vec2 &pos) const
{
	Vec2 start_pos = get_chesspiece_start_pos();
	return start_pos +
		Vec2(pos.x * kChessPieceWidth + pos.x * kInterval + kChessPieceWidth / 2,
		pos.y * kChessPieceHeight + pos.y * kInterval + kChessPieceHeight / 2);
}

// 世界坐标转换棋盘坐标
Vec2 Checkerboard::convert_to_checkerboard_space(const Vec2 &pos) const
{
	Vec2 start_pos = get_chesspiece_start_pos();
	int col = (pos.x - start_pos.x - kInterval) / kChessPieceWidth;
	int row = (pos.y - start_pos.y - kInterval) / kChessPieceHeight;
	return Vec2(col , row);
}

bool Checkerboard::onTouchBegan(Touch *touch, Event *unused_event)
{
	if (action_lock_)
	{
		return false;
	}
	else
	{
		touch_begin_pos_ = touch->getLocation();
		return true;
	}
}

void Checkerboard::onTouchMoved(Touch *touch, Event *unused_event)
{
	// 防止手滑
	if (abs(touch->getLocation().distance(touch_begin_pos_)) >= kMinMoveDistance)
	{
		real_move_ = true;
	}
}

void Checkerboard::onTouchEnded(Touch *touch, Event *unused_event)
{
	if (real_move_)
	{
		real_move_ = false;
		auto src = convert_to_checkerboard_space(touch_begin_pos_);
		auto goes = convert_to_checkerboard_space(touch->getLocation());
		CCLOG("Vec2(%f, %f) Move To Vec2(%f, %f)", src.x, src.y, goes.x, goes.y);
		LogicHandle::instance()->move_chess_piece(src, goes);
	}
}

void Checkerboard::onTouchCancelled(Touch *touch, Event *unused_event)
{
	onTouchEnded(touch, unused_event);
}

// 获取棋子精灵
Sprite* Checkerboard::get_chesspiece_sprite(const Vec2 &pos)
{
	int index = pos.y * LogicHandle::kCheckerboardColNum + pos.x;
	return chesspiece_sprite_[index];
}

// 移动棋子
void Checkerboard::on_move_chesspiece(const Vec2 &source, const Vec2 &target)
{
	Sprite *chess_piece = get_chesspiece_sprite(source);
	assert(chess_piece != nullptr);
	if (chess_piece != nullptr)
	{
		Vec2 world_pos = convert_to_world_space(target);
		chess_piece->runAction(Sequence::create(
			MoveTo::create(0.1f, world_pos),
			CallFunc::create([=]()
		{
			int s_index = source.y * LogicHandle::kCheckerboardColNum + source.x;
			int t_index = target.y * LogicHandle::kCheckerboardColNum + target.x;
			std::swap(chesspiece_sprite_[s_index], chesspiece_sprite_[t_index]);
		}),
			CallFunc::create(std::bind(&Checkerboard::finished_action, this)),
			nullptr));
	}
}

// 吃掉棋子
void Checkerboard::on_kill_chesspiece(const Vec2 &source, const Vec2 &target)
{

}

// 更新动作
void Checkerboard::update_action()
{
	perform_action();
}

// 执行动作
void Checkerboard::perform_action()
{
	if (!action_lock_)
	{
		LogicHandle::EventDetails action = LogicHandle::instance()->take_event_info();
		if (action.type != LogicHandle::EventType::None)
		{
			action_lock_ = true;

			// 移动
			if (action.type == LogicHandle::EventType::Moved)
			{
				on_move_chesspiece(action.source, action.target);
			}
			// 吃子
			else if (action.type == LogicHandle::EventType::Killed)
			{
				on_kill_chesspiece(action.source, action.target);
			}
		}
	}
}

// 完成动作
void Checkerboard::finished_action()
{
	action_lock_ = false;
	perform_action();
}
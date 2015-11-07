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
	, touch_lock_(false)
{

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
	for (size_t i = 0; i < pieces_.size(); ++i)
	{
		int row = i / LogicHandle::kCheckerboardRowNum;
		int col = i % LogicHandle::kCheckerboardRowNum;
		auto layer = LayerColor::create();
		layer->setContentSize(Size(kChessPieceWidth, kChessPieceHeight));
		layer->setPosition(start_pos + Vec2(col * kChessPieceWidth + col * kInterval, row * kChessPieceHeight + row * kInterval));
		pieces_[i] = layer;
		addChild(layer);
	}

	// 刷新棋盘
	refresh_checkerboard();

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
	std::default_random_engine generator(time(nullptr));
	std::uniform_int_distribution<int> dis(0, g_seven_colors.size() - 1);
	const Color4B color = g_seven_colors[dis(generator)];

	for (size_t i = 0; i < pieces_.size(); ++i)
	{
		int index = (i % LogicHandle::kCheckerboardRowNum) % 2;
		if (pieces_[i] != nullptr)
		{
			pieces_[i]->initWithColor(color, kChessPieceWidth, kChessPieceHeight);
		}
	}

	// 绘制棋子
	Vec2 start_pos = get_chesspiece_start_pos();
	LogicHandle::instance()->visit_checkerboard([&](const cocos2d::Vec2 &pos, int value)
	{
		if (value != 0)
		{
			Vec2 new_pos(start_pos + Vec2(pos.x * kChessPieceWidth + pos.x * kInterval + kChessPieceWidth / 2,
				pos.y * kChessPieceHeight + pos.y * kInterval + kChessPieceHeight / 2));
			Sprite *sp = Sprite::create(value == 1 ? "whiteplay.png" : "blackplay.png");
			sp->setPosition(new_pos);
			addChild(sp);
		}
	});
}

// 世界坐标转换棋盘坐标
Vec2 Checkerboard::convert_to_checkerboard_space(const cocos2d::Vec2 &pos) const
{
	Vec2 start_pos = get_chesspiece_start_pos();
	int col = (pos.x - start_pos.x - kInterval) / kChessPieceWidth;
	int row = (pos.y - start_pos.y - kInterval) / kChessPieceHeight;
	return Vec2(col , row);
}

bool Checkerboard::onTouchBegan(Touch *touch, Event *unused_event)
{
	if (touch_lock_)
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
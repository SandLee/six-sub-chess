﻿#include "CheckerboardLayer.h"

#include <cmath>
#include "Language.h"
#include "GameScene.h"
#include "VisibleRect.h"
#include "ColorGenerator.h"
using namespace cocos2d;


namespace
{
	// 棋子正常显示层级
	const int kNormalChessPieceZOrder = 1;

	// 棋子选中显示层级
	const int kSelectedChessPieceZOrder = 2;

	Vec2 ToCocos2DVec2(const FVec2 &pos)
	{
		return Vec2(pos.x, pos.y);
	}

	FVec2 ToCheckerboardVec2(const Vec2 &pos)
	{
		return FVec2(static_cast<int>(pos.x), static_cast<int>(pos.y));
	}
}

CheckerboardLayer::CheckerboardLayer(SingleLogic *logic)
	: logic_(logic)
	, action_lock_(false)
	, action_read_pos_(0)
	, operation_lock_(true)
	, selected_chesspiece_(nullptr)
	, chesspiece_type_(FChessPieceType::NONE)
{
	for (size_t i = 0; i < chesspiece_sprite_.size(); ++i)
	{
		chesspiece_sprite_[i] = nullptr;
	}
}

CheckerboardLayer::~CheckerboardLayer()
{

}

CheckerboardLayer* CheckerboardLayer::create(SingleLogic *logic)
{
	assert(logic != nullptr);
	CheckerboardLayer *ret = new (std::nothrow) CheckerboardLayer(logic);
	if (ret && ret->init())
	{
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool CheckerboardLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	// 生成地板
	Vec2 start_pos = getChesspieceStartPos();
	for (size_t i = 0; i < color_floor_.size(); ++i)
	{
		int row = i / kCheckerboardColNum;
		int col = i % kCheckerboardColNum;
		auto layer = LayerColor::create();
		layer->setContentSize(Size(kChessPieceWidth, kChessPieceHeight));
		layer->setPosition(start_pos + Vec2(col * kChessPieceWidth + col * kInterval, row * kChessPieceHeight + row * kInterval));
		color_floor_[i] = layer;
		addChild(layer);
	}

	const Color4B color(ColorGenerator::instance()->rand());
	for (size_t i = 0; i < color_floor_.size(); ++i)
	{
		int index = (i % kCheckerboardRowNum) % 2;
		if (color_floor_[i] != nullptr)
		{
			color_floor_[i]->initWithColor(color, kChessPieceWidth, kChessPieceHeight);
		}
	}

	// 开启触摸
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(CheckerboardLayer::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(CheckerboardLayer::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(CheckerboardLayer::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(CheckerboardLayer::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	// 接收逻辑更新通知
	logic_->addActionUpdateCallback(std::bind(&CheckerboardLayer::updateAction, this));

	return true;
}

// 重置
void CheckerboardLayer::reset(FChessPieceType type)
{
	assert(logic_ != nullptr);
	assert(type != FChessPieceType::NONE);
	action_lock_ = false;
	action_read_pos_ = 0;
	operation_lock_ = true;
	chesspiece_type_ = type;
}

// 获取棋子开始位置
Vec2 CheckerboardLayer::getChesspieceStartPos() const
{
	Vec2 start_pos;
	start_pos.x = VisibleRect::center().x - (kChessPieceWidth * kCheckerboardColNum + kInterval * (kCheckerboardColNum - 1)) / 2;
	start_pos.y = VisibleRect::center().y - (kChessPieceHeight * kCheckerboardRowNum + kInterval * (kCheckerboardRowNum - 1)) / 2;
	return start_pos;
}

// 获取棋子类型
FChessPieceType CheckerboardLayer::getChesspieceType() const
{
	return chesspiece_type_;
}

// 棋盘坐标转视图坐标
Vec2 CheckerboardLayer::convertToViewSpace(const Vec2 &pos) const
{
	if (getChesspieceType() == FChessPieceType::WHITE)
	{
		return pos;
	}
	else
	{
		Vec2 view_pos = pos;
		view_pos.y = std::abs(view_pos.y - 3);
		return view_pos;
	}
}

// 棋盘坐标转换到世界坐标
Vec2 CheckerboardLayer::convertToWorldSpace(const Vec2 &pos) const
{
	Vec2 view_pos = convertToViewSpace(pos);
	Vec2 start_pos = getChesspieceStartPos();
	return start_pos +
		Vec2(view_pos.x * kChessPieceWidth + view_pos.x * kInterval + kChessPieceWidth / 2,
		view_pos.y * kChessPieceHeight + view_pos.y * kInterval + kChessPieceHeight / 2);
}

// 世界坐标转换棋盘坐标
Vec2 CheckerboardLayer::convertToCheckerboardSpace(const Vec2 &pos) const
{
	Vec2 start_pos = getChesspieceStartPos();
	int col = (pos.x - start_pos.x - kInterval) / kChessPieceWidth;
	int row = (pos.y - start_pos.y - kInterval) / kChessPieceHeight;
	col = col < 0 ? 0 : col >= kCheckerboardColNum ? kCheckerboardColNum - 1 : col;
	row = row < 0 ? 0 : row >= kCheckerboardRowNum ? kCheckerboardRowNum - 1 : row;
	return convertToViewSpace(Vec2(col, row));
}

// 获取棋子精灵
Sprite* CheckerboardLayer::getChesspieceSprite(const Vec2 &pos)
{
	int index = pos.y * kCheckerboardColNum + pos.x;
	return chesspiece_sprite_[index];
}

// 刷新棋盘
void CheckerboardLayer::refreshCheckerboard()
{
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
	Vec2 start_pos = getChesspieceStartPos();
	logic_->visitCheckerboard([&](const FVec2 &pos, int value)
	{
		if (value != 0)
		{
			Sprite *chesspiece = nullptr;
			if (free_sprite_.empty())
			{
				chesspiece = Sprite::create(value == 1 ? "whiteplay.png" : "blackplay.png");
				chesspiece->setScale(Director::getInstance()->getContentScaleFactor());
				addChild(chesspiece);
			}
			else
			{
				chesspiece = free_sprite_.back();
				free_sprite_.pop_back();
				Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(value == 1 ? "whiteplay.png" : "blackplay.png");
				chesspiece->setTexture(texture);
				chesspiece->setTextureRect(Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
				chesspiece->setVisible(true);
				chesspiece->setOpacity(255);
			}
			chesspiece->setPosition(convertToWorldSpace(ToCocos2DVec2(pos)));
			chesspiece->setLocalZOrder(kNormalChessPieceZOrder);
			int index = pos.y * kCheckerboardColNum + pos.x;
			assert(chesspiece_sprite_[index] == nullptr);
			chesspiece_sprite_[index] = chesspiece;
		}
	});
}

// 移动棋子
void CheckerboardLayer::onMoveChesspiece(const Vec2 &source, const Vec2 &target)
{
	Sprite *chesspiece = getChesspieceSprite(source);
	assert(chesspiece != nullptr);
	if (chesspiece != nullptr)
	{
		Vec2 world_pos = convertToWorldSpace(target);
		chesspiece->runAction(Sequence::create(
			MoveTo::create(0.1f, world_pos),
			CallFunc::create([=]()
		{
			int s_index = source.y * kCheckerboardColNum + source.x;
			int t_index = target.y * kCheckerboardColNum + target.x;
			std::swap(chesspiece_sprite_[s_index], chesspiece_sprite_[t_index]);
		}),
			CallFunc::create(std::bind(&CheckerboardLayer::actionFinished, this)),
			nullptr));
	}
}

// 吃掉棋子
void CheckerboardLayer::onKillChesspiece(const Vec2 &source, const Vec2 &target)
{
	Sprite *chesspiece = getChesspieceSprite(target);
	assert(chesspiece != nullptr);
	if (chesspiece != nullptr)
	{
		chesspiece->runAction(Sequence::create(
			FadeOut::create(0.5f),
			CallFunc::create([=]()
		{
			int index = target.y * kCheckerboardColNum + target.x;
			free_sprite_.push_back(chesspiece_sprite_[index]);
			chesspiece_sprite_[index] = nullptr;
		}),
			CallFunc::create(std::bind(&CheckerboardLayer::actionFinished, this)),
			nullptr));
	}
}

// 更新动作
void CheckerboardLayer::updateAction()
{
	runPlayerAction();
}

// 执行动作
void CheckerboardLayer::runPlayerAction()
{
	if (!action_lock_)
	{
		FAction action = logic_->getActionFromQueue(action_read_pos_);
		if (action.type != FActionType::NONE)
		{
			action_lock_ = true;

			switch (action.type)
			{
				// 开始游戏
				case FActionType::START:
				{
					refreshCheckerboard();
					runAction(Sequence::create(CallFunc::create(std::bind(&CheckerboardLayer::actionFinished, this)), nullptr));
					break;
				}
				// 移动棋子
				case FActionType::MOVED:
				{
					// 自己的移动操作实时处理，不通过逻辑处理器进行
					if (action.chess_type != getChesspieceType())
					{
						onMoveChesspiece(ToCocos2DVec2(action.source), ToCocos2DVec2(action.target));
					}
					else
					{
						runAction(Sequence::create(CallFunc::create(std::bind(&CheckerboardLayer::actionFinished, this)), nullptr));
					}
					break;
				}
				// 杀掉棋子
				case FActionType::KILLED:
				{
					onKillChesspiece(ToCocos2DVec2(action.source), ToCocos2DVec2(action.target));
					break;
				}
				// 玩家待机
				case FActionType::STANDBY:
				{
					operation_lock_ = action.chess_type == getChesspieceType();
					runAction(Sequence::create(CallFunc::create(std::bind(&CheckerboardLayer::actionFinished, this)), nullptr));
					GameScene *parent = dynamic_cast<GameScene *>(getParent());
					parent->setGameTips(getChesspieceType() == action.chess_type ? lang("wait") : lang("play_chess"));
					break;
				}
				// 游戏结束
				case FActionType::GAMEOVER:
				{
					operation_lock_ = true;
					runAction(Sequence::create(CallFunc::create(std::bind(&CheckerboardLayer::actionFinished, this)), nullptr));
					GameScene *parent = dynamic_cast<GameScene *>(getParent());
					parent->setGameTips(getChesspieceType() == action.chess_type ? lang("win") : lang("lost"));
					break;
				}
				default:
					actionFinished();
			}
		}
	}
}

// 完成动作
void CheckerboardLayer::actionFinished()
{
	++action_read_pos_;
	action_lock_ = false;
	runPlayerAction();
}

bool CheckerboardLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
	if (!action_lock_ && !operation_lock_)
	{
		Vec2 chesspiece_pos = convertToCheckerboardSpace(touch->getLocation());
		if (logic_->getChesspieceType(ToCheckerboardVec2(chesspiece_pos)) == getChesspieceType())
		{
			auto chesspiece = getChesspieceSprite(chesspiece_pos);
			if (chesspiece != nullptr)
			{
				selected_chesspiece_ = chesspiece;
				selected_chesspiece_->setLocalZOrder(kSelectedChessPieceZOrder);
				touch_begin_pos_ = touch->getLocation();
				return true;
			}
		}
	}
	return false;
}

void CheckerboardLayer::onTouchMoved(Touch *touch, Event *unused_event)
{
	// 移动棋子
	assert(selected_chesspiece_ != nullptr);
	if (selected_chesspiece_ != nullptr)
	{
		selected_chesspiece_->setPosition(touch->getLocation());
	}
}

void CheckerboardLayer::onTouchEnded(Touch *touch, Event *unused_event)
{
	if (selected_chesspiece_ != nullptr)
	{
		auto source = convertToCheckerboardSpace(touch_begin_pos_);
		auto target = convertToCheckerboardSpace(touch->getLocation());

		if (logic_->isAdjacent(ToCheckerboardVec2(source), ToCheckerboardVec2(target)) &&
			!logic_->isValidChesspiece(ToCheckerboardVec2(target)) &&
			!operation_lock_)
		{
			selected_chesspiece_->setPosition(convertToWorldSpace(target));
			logic_->moveChesspiece(ToCheckerboardVec2(source), ToCheckerboardVec2(target));

			int s_index = source.y * kCheckerboardColNum + source.x;
			int t_index = target.y * kCheckerboardColNum + target.x;
			std::swap(chesspiece_sprite_[s_index], chesspiece_sprite_[t_index]);
		}
		else
		{
			selected_chesspiece_->setPosition(convertToWorldSpace(source));
		}

		selected_chesspiece_->setLocalZOrder(kNormalChessPieceZOrder);
		selected_chesspiece_ = nullptr;
	}
}

void CheckerboardLayer::onTouchCancelled(Touch *touch, Event *unused_event)
{
	onTouchEnded(touch, unused_event);
}
#include "CheckerboardLayer.h"

#include <random>
#include "VisibleRect.h"
using namespace cocos2d;


namespace
{
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

	// 棋子正常显示层级
	const int kNormalChessPieceZOrder = 1;

	// 棋子选中显示层级
	const int kSelectedChessPieceZOrder = 2;

	Vec2 ToCocos2DVec2(const GameLogic::Vec2 &pos)
	{
		return Vec2(pos.x, pos.y);
	}

	GameLogic::Vec2 ToCheckerboardVec2(const Vec2 &pos)
	{
		return GameLogic::Vec2(static_cast<int>(pos.x), static_cast<int>(pos.y));
	}
}

CheckerboardLayer::CheckerboardLayer()
	: logic_(nullptr)
	, action_lock_(false)
	, operation_lock_(true)
	, selected_chesspiece_(nullptr)
	, chesspiece_type_(GameLogic::ChessPieceType::NONE)
{
	for (size_t i = 0; i < chesspiece_sprite_.size(); ++i)
	{
		chesspiece_sprite_[i] = nullptr;
	}
}

CheckerboardLayer::~CheckerboardLayer()
{

}

bool CheckerboardLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	// 生成地板
	Vec2 start_pos = get_chesspiece_start_pos();
	for (size_t i = 0; i < color_floor_.size(); ++i)
	{
		int row = i / GameLogic::kCheckerboardRowNum;
		int col = i % GameLogic::kCheckerboardRowNum;
		auto layer = LayerColor::create();
		layer->setContentSize(Size(kChessPieceWidth, kChessPieceHeight));
		layer->setPosition(start_pos + Vec2(col * kChessPieceWidth + col * kInterval, row * kChessPieceHeight + row * kInterval));
		color_floor_[i] = layer;
		addChild(layer);
	}

	std::default_random_engine generator(time(nullptr));
	std::uniform_int_distribution<int> dis(0, g_seven_colors.size() - 1);
	const Color4B color = g_seven_colors[dis(generator)];

	for (size_t i = 0; i < color_floor_.size(); ++i)
	{
		int index = (i % GameLogic::kCheckerboardRowNum) % 2;
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
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

// 是否已设定逻辑
bool CheckerboardLayer::has_logic() const
{
	return logic_ != nullptr;
}

// 生成棋盘
void CheckerboardLayer::generate_chessboard(GameLogic::ChessPieceType type, GameLogic *logic)
{
	assert(logic != nullptr);
	assert(type != GameLogic::ChessPieceType::NONE);
	logic_ = logic;
	chesspiece_type_ = type;

	if (has_logic())
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
		Vec2 start_pos = get_chesspiece_start_pos();
		logic_->visit_checkerboard([&](const GameLogic::Vec2 &pos, int value)
		{
			if (value != 0)
			{
				Sprite *chess_piece = nullptr;
				if (free_sprite_.empty())
				{
					chess_piece = Sprite::create(value == 1 ? "whiteplay.png" : "blackplay.png");
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
				chess_piece->setPosition(convert_to_world_space(ToCocos2DVec2(pos)));
				chess_piece->setLocalZOrder(kNormalChessPieceZOrder);
				int index = pos.y * GameLogic::kCheckerboardColNum + pos.x;
				assert(chesspiece_sprite_[index] == nullptr);
				chesspiece_sprite_[index] = chess_piece;
			}
		});

		// 接收棋盘更新通知
		logic_->add_action_update_callback(std::bind(&CheckerboardLayer::update_action, this));
	}
}

// 获取棋子开始位置
Vec2 CheckerboardLayer::get_chesspiece_start_pos() const
{
	Vec2 start_pos;
	start_pos.x = VisibleRect::center().x - (kChessPieceWidth * GameLogic::kCheckerboardColNum + kInterval * (GameLogic::kCheckerboardColNum - 1)) / 2;
	start_pos.y = VisibleRect::center().y - (kChessPieceHeight * GameLogic::kCheckerboardRowNum + kInterval * (GameLogic::kCheckerboardRowNum - 1)) / 2;
	return start_pos;
}

// 获取棋子类型
GameLogic::ChessPieceType CheckerboardLayer::get_chesspiece_type() const
{
	return chesspiece_type_;
}

// 棋盘坐标转视图坐标
Vec2 CheckerboardLayer::convert_to_view_space(const Vec2 &pos) const
{
	if (get_chesspiece_type() == GameLogic::WHITE)
	{
		return pos;
	}
	else
	{
		Vec2 view_pos = pos;
		view_pos.y = abs(view_pos.y - 3);
		return view_pos;
	}
}

// 棋盘坐标转换到世界坐标
Vec2 CheckerboardLayer::convert_to_world_space(const Vec2 &pos) const
{
	Vec2 view_pos = convert_to_view_space(pos);
	Vec2 start_pos = get_chesspiece_start_pos();
	return start_pos +
		Vec2(view_pos.x * kChessPieceWidth + view_pos.x * kInterval + kChessPieceWidth / 2,
		view_pos.y * kChessPieceHeight + view_pos.y * kInterval + kChessPieceHeight / 2);
}

// 世界坐标转换棋盘坐标
Vec2 CheckerboardLayer::convert_to_checkerboard_space(const Vec2 &pos) const
{
	Vec2 start_pos = get_chesspiece_start_pos();
	int col = (pos.x - start_pos.x - kInterval) / kChessPieceWidth;
	int row = (pos.y - start_pos.y - kInterval) / kChessPieceHeight;
	col = col < 0 ? 0 : col >= GameLogic::kCheckerboardColNum ? GameLogic::kCheckerboardColNum - 1 : col;
	row = row < 0 ? 0 : row >= GameLogic::kCheckerboardRowNum ? GameLogic::kCheckerboardRowNum - 1 : row;
	return convert_to_view_space(Vec2(col, row));
}

// 获取棋子精灵
Sprite* CheckerboardLayer::get_chesspiece_sprite(const Vec2 &pos)
{
	int index = pos.y * GameLogic::kCheckerboardColNum + pos.x;
	return chesspiece_sprite_[index];
}

// 移动棋子
void CheckerboardLayer::on_move_chesspiece(const Vec2 &source, const Vec2 &target)
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
			int s_index = source.y * GameLogic::kCheckerboardColNum + source.x;
			int t_index = target.y * GameLogic::kCheckerboardColNum + target.x;
			std::swap(chesspiece_sprite_[s_index], chesspiece_sprite_[t_index]);
		}),
			CallFunc::create(std::bind(&CheckerboardLayer::finished_action, this)),
			nullptr));
	}
}

// 吃掉棋子
void CheckerboardLayer::on_kill_chesspiece(const Vec2 &source, const Vec2 &target)
{
	Sprite *chess_piece = get_chesspiece_sprite(target);
	assert(chess_piece != nullptr);
	if (chess_piece != nullptr)
	{
		chess_piece->setVisible(false);
		chess_piece->runAction(Sequence::create(
			CallFunc::create([=]()
		{
			int index = target.y * GameLogic::kCheckerboardColNum + target.x;
			free_sprite_.push_back(chesspiece_sprite_[index]);
			chesspiece_sprite_[index] = nullptr;
		}),
			CallFunc::create(std::bind(&CheckerboardLayer::finished_action, this)),
			nullptr));
	}
}

// 更新动作
void CheckerboardLayer::update_action()
{
	perform_action();
}

// 执行动作
void CheckerboardLayer::perform_action()
{
	if (!action_lock_ && has_logic())
	{
		GameLogic::Action action = logic_->take_action_from_queue();
		if (action.type != GameLogic::ActionType::NONE)
		{
			action_lock_ = true;

			// 移动
			if (action.type == GameLogic::ActionType::MOVED)
			{
				// 自己的移动操作实时处理，不通过逻辑处理器进行
				if (action.chess_type != get_chesspiece_type())
				{
					on_move_chesspiece(ToCocos2DVec2(action.source), ToCocos2DVec2(action.target));
				}
				else
				{
					runAction(Sequence::create(CallFunc::create(std::bind(&CheckerboardLayer::finished_action, this)), nullptr));
				}
			}
			// 吃子
			else if (action.type == GameLogic::ActionType::KILLED)
			{
				on_kill_chesspiece(ToCocos2DVec2(action.source), ToCocos2DVec2(action.target));
			}
			// 待机
			else if (action.type == GameLogic::ActionType::STANDBY)
			{
				operation_lock_ = action.chess_type == get_chesspiece_type();
				runAction(Sequence::create(CallFunc::create(std::bind(&CheckerboardLayer::finished_action, this)), nullptr));
			}
		}
	}
}

// 完成动作
void CheckerboardLayer::finished_action()
{
	action_lock_ = false;
	perform_action();
}

bool CheckerboardLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
	if (has_logic() && !action_lock_ && !operation_lock_)
	{
		Vec2 chesspiece_pos = convert_to_checkerboard_space(touch->getLocation());
		if (logic_->get_chesspiece_type(ToCheckerboardVec2(chesspiece_pos)) == get_chesspiece_type())
		{
			auto chesspiece = get_chesspiece_sprite(chesspiece_pos);
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
	if (selected_chesspiece_ != nullptr )
	{
		selected_chesspiece_->setPosition(touch->getLocation());
	}
}

void CheckerboardLayer::onTouchEnded(Touch *touch, Event *unused_event)
{
	if (has_logic() && selected_chesspiece_ != nullptr)
	{
		auto source = convert_to_checkerboard_space(touch_begin_pos_);
		auto target = convert_to_checkerboard_space(touch->getLocation());

		if (logic_->is_adjacent(ToCheckerboardVec2(source), ToCheckerboardVec2(target)) &&
			!logic_->is_valid_chess_piece(ToCheckerboardVec2(target)) &&
			!operation_lock_)
		{
			selected_chesspiece_->setPosition(convert_to_world_space(target));
			logic_->move_chess_piece(ToCheckerboardVec2(source), ToCheckerboardVec2(target));

			int s_index = source.y * GameLogic::kCheckerboardColNum + source.x;
			int t_index = target.y * GameLogic::kCheckerboardColNum + target.x;
			std::swap(chesspiece_sprite_[s_index], chesspiece_sprite_[t_index]);
		}
		else
		{
			selected_chesspiece_->setPosition(convert_to_world_space(source));
		}

		selected_chesspiece_->setLocalZOrder(kNormalChessPieceZOrder);
		selected_chesspiece_ = nullptr;
	}
}

void CheckerboardLayer::onTouchCancelled(Touch *touch, Event *unused_event)
{
	onTouchEnded(touch, unused_event);
}
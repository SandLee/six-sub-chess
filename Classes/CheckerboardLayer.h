#ifndef __CHECKERBOARD_H__
#define __CHECKERBOARD_H__

#include <array>
#include "cocos2d.h"
#include "GameLogic.h"

class CheckerboardLayer : public cocos2d::Layer
{
public:
	static const int kInterval = 5;					// 棋盘间距
	static const int kChessPieceWidth = 120;		// 棋子宽度
	static const int kChessPieceHeight = 120;		// 棋子高度
	static const int kChessspieceSum = GameLogic::kCheckerboardRowNum * GameLogic::kCheckerboardColNum;

public:
	CheckerboardLayer();

	~CheckerboardLayer();

	virtual bool init() override;

	CREATE_FUNC(CheckerboardLayer);

public:
	/**
	 * 是否已设定逻辑
	 */
	bool has_logic() const;

	/**
	 * 获取棋子开始位置
	 */
	cocos2d::Vec2 get_chesspiece_start_pos() const;

	/**
	 * 获取棋子类型
	 */
	GameLogic::ChessPieceType get_chesspiece_type() const;

	/**
	 * 生成棋盘
	 */
	void generate_chessboard(GameLogic::ChessPieceType type, GameLogic *logic);

	/**
	 * 棋盘坐标转换到世界坐标
	 */
	cocos2d::Vec2 convert_to_world_space(const cocos2d::Vec2 &pos) const;

	/**
	 * 世界坐标转换棋盘坐标
	 */
	cocos2d::Vec2 convert_to_checkerboard_space(const cocos2d::Vec2 &pos) const;

	/**
	 * 更新动作
	 */
	void update_action();

	/**
	 * 执行动作
	 */
	void perform_action();

	/**
	 * 完成动作
	 */
	void finished_action();

public:
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
	
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
	
	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

private:
	/**
	 * 棋盘坐标转视图坐标
	 */
	cocos2d::Vec2 convert_to_view_space(const cocos2d::Vec2 &pos) const;

	/**
	 * 获取棋子精灵
	 * @param pos 棋盘坐标系位置
	 */
	cocos2d::Sprite* get_chesspiece_sprite(const cocos2d::Vec2 &pos);

	/**
	 * 移动棋子
	 */
	void on_move_chesspiece(const cocos2d::Vec2 &source, const cocos2d::Vec2 &target);

	/**
	 * 吃掉棋子
	 */
	void on_kill_chesspiece(const cocos2d::Vec2 &source, const cocos2d::Vec2 &target);

private:
	GameLogic*											logic_;
	bool												action_lock_;
	GameLogic::ChessPieceType							chesspiece_type_;
	cocos2d::Sprite*									selected_chesspiece_;
	cocos2d::Vec2										touch_begin_pos_;
	std::vector<cocos2d::Sprite *>						free_sprite_;
	std::array<cocos2d::Sprite *, kChessspieceSum>		chesspiece_sprite_;
	std::array<cocos2d::LayerColor*, kChessspieceSum>	color_floor_;
};

#endif
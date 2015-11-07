#ifndef __CHECKERBOARD_H__
#define __CHECKERBOARD_H__

#include <array>
#include "LogicHandle.h"


class Checkerboard : public cocos2d::Layer
{
public:
	static const int kInterval = 5;					// 棋盘间距
	static const int kChessPieceWidth = 120;		// 棋子宽度
	static const int kChessPieceHeight = 120;		// 棋子高度
	static const int kChessspieceSum = LogicHandle::kCheckerboardRowNum * LogicHandle::kCheckerboardColNum;

public:
	Checkerboard();

	~Checkerboard();

	virtual bool init() override;

	CREATE_FUNC(Checkerboard);

public:
	/**
	 * 刷新棋盘
	 */
	void refresh_checkerboard();

	/**
	 * 获取棋子开始位置
	 */
	cocos2d::Vec2 get_chesspiece_start_pos() const;

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
	 * 获取棋子精灵
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
	bool												real_move_;
	bool												action_lock_;
	cocos2d::Vec2										touch_begin_pos_;
	std::vector<cocos2d::Sprite *>						free_sprite_;
	std::array<cocos2d::Sprite *, kChessspieceSum>		chesspiece_sprite_;
	std::array<cocos2d::LayerColor*, kChessspieceSum>	color_floor_;
};

#endif
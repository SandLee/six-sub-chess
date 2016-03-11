#ifndef __CHECKERBOARD_H__
#define __CHECKERBOARD_H__

#include <array>
#include "cocos2d.h"
#include "SingleLogic.h"

class CheckerboardLayer : public cocos2d::Layer
{
public:
	static const int kInterval = 5;					// 棋盘间距
	static const int kChessPieceWidth = 120;		// 棋子宽度
	static const int kChessPieceHeight = 120;		// 棋子高度
	static const int kChessspieceSum = kCheckerboardRowNum * kCheckerboardColNum;

public:
	CheckerboardLayer(SingleLogic *logic);

	~CheckerboardLayer();

	virtual bool init() override;

	static CheckerboardLayer* create(SingleLogic *logic);

public:
	/**
	 * 重置
	 */
	void reset(FChessPieceType type);

	/**
	 * 获取棋子开始位置
	 */
	cocos2d::Vec2 getChesspieceStartPos() const;

	/**
	 * 获取棋子类型
	 */
	FChessPieceType getChesspieceType() const;

	/**
	 * 棋盘坐标转换到世界坐标
	 */
	cocos2d::Vec2 convertToWorldSpace(const cocos2d::Vec2 &pos) const;

	/**
	 * 世界坐标转换棋盘坐标
	 */
	cocos2d::Vec2 convertToCheckerboardSpace(const cocos2d::Vec2 &pos) const;

	/**
	 * 更新动作
	 */
	void updateAction();

	/**
	 * 执行动作
	 */
	void runPlayerAction();

	/**
	 * 完成动作
	 */
	void actionFinished();

public:
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
	
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
	
	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

private:
	/**
	 * 刷新棋盘
	 */
	void refreshCheckerboard();

	/**
	 * 棋盘坐标转视图坐标
	 */
	cocos2d::Vec2 convertToViewSpace(const cocos2d::Vec2 &pos) const;

	/**
	 * 获取棋子精灵
	 */
	cocos2d::Sprite* getChesspieceSprite(const cocos2d::Vec2 &pos);

	/**
	 * 移动棋子
	 */
	void onMoveChesspiece(const cocos2d::Vec2 &source, const cocos2d::Vec2 &target);

	/**
	 * 吃掉棋子
	 */
	void onKillChesspiece(const cocos2d::Vec2 &source, const cocos2d::Vec2 &target);

private:
	SingleLogic*										logic_;
	bool												action_lock_;
	bool												operation_lock_;
	int													action_read_pos_;
	FChessPieceType										chesspiece_type_;
	cocos2d::Sprite*									selected_chesspiece_;
	cocos2d::Vec2										touch_begin_pos_;
	std::vector<cocos2d::Sprite *>						free_sprite_;
	std::array<cocos2d::Sprite *, kChessspieceSum>		chesspiece_sprite_;
	std::array<cocos2d::LayerColor*, kChessspieceSum>	color_floor_;
};

#endif
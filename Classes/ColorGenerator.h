#ifndef __COLORGENERATOR_H__
#define __COLORGENERATOR_H__

#include "cocos2d.h"
#include "Singleton.h"

class ColorGenerator : public Singleton < ColorGenerator >
{
	SINGLETON(ColorGenerator);

public:
	/**
	 * 随机生成一种颜色
	 */
	cocos2d::Color3B rand() const;

	/**
	 * 最多随机生成不重复颜色的数量
	 */
	unsigned int max_rand_not_repeat_num() const;

	/**
	 * 随机生成几种不重复的颜色（最多支持七种）
	 */
	std::vector<cocos2d::Color3B> rand_not_repeat(unsigned int num) const;
};

#endif
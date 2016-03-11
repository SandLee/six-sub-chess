#include "ColorGenerator.h"

#include <set>
#include <ctime>
using namespace cocos2d;


// 七色
static const std::vector<Color3B> g_seven_colors =
{
	Color3B(255, 0, 0),
	Color3B(255, 165, 0),
	Color3B(255, 255, 0),
	Color3B(0, 128, 0),
	Color3B(0, 255, 255),
	Color3B(0, 0, 255),
	Color3B(79, 47, 79)
};

ColorGenerator::ColorGenerator()
	: generator_(time(nullptr))
{

}

ColorGenerator::~ColorGenerator()
{

}

// 随机生成一种颜色
Color3B ColorGenerator::rand() const
{
	std::uniform_int_distribution<int> dis(0, g_seven_colors.size() - 1);
	return g_seven_colors[dis(generator_)];
}

// 最多随机生成不重复颜色的数量
unsigned int ColorGenerator::maxRandNotRepeatNum() const
{
	return g_seven_colors.size();
}

// 随机生成几种不重复的颜色（最多支持七种）
std::vector<cocos2d::Color3B> ColorGenerator::randNotRepeat(unsigned int num) const
{
	assert(num <= maxRandNotRepeatNum());
	if (num > maxRandNotRepeatNum())
	{
		num = maxRandNotRepeatNum();
	}
	std::vector<cocos2d::Color3B> color_array;
	std::vector<Color3B> seven_colors = g_seven_colors;
	for (unsigned int i = 0; i < num && !seven_colors.empty(); ++i)
	{
		std::uniform_int_distribution<int> dis(0, seven_colors.size() - 1);
		std::swap(seven_colors[dis(generator_)], seven_colors.back());
		color_array.push_back(seven_colors.back());
		seven_colors.pop_back();
	}
	return color_array;
}
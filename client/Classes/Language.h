#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include <string>
#include <unordered_map>
#include "cocos2d.h"
#include "Singleton.h"

class Language : public Singleton < Language >
{
	SINGLETON(Language);
	friend std::string lang(const std::string &text);

public:
	void setLanguage(cocos2d::LanguageType type);

private:
	std::unordered_map<std::string, std::string> corresponding_table_;
};

std::string lang(const std::string &text);

#endif
#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include <string>
#include <unordered_map>
#include "cocos2d.h"
#include "Singleton.h"

class Language : public Singleton < Language >
{
	SINGLETON(Language);
	friend const std::string& lang(const std::string &text);

public:
	void set_language(cocos2d::LanguageType type);

private:
	std::unordered_map<std::string, std::string> corresponding_table_;
};

const std::string& lang(const std::string &text);

#endif
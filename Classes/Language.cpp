#include "Language.h"
#include "json/document.h"
using namespace cocos2d;


static const std::map<LanguageType, const char *> g_language_map =
{
	{ LanguageType::ENGLISH, "english" },
	{ LanguageType::CHINESE, "chinese" },
	{ LanguageType::FRENCH, "french" },
	{ LanguageType::ITALIAN, "italian" },
	{ LanguageType::GERMAN, "german" },
	{ LanguageType::SPANISH, "spanish" },
	{ LanguageType::DUTCH, "dutch" },
	{ LanguageType::RUSSIAN, "russian" },
	{ LanguageType::KOREAN, "korean" },
	{ LanguageType::JAPANESE, "japanese" },
	{ LanguageType::HUNGARIAN, "hungarian" },
	{ LanguageType::PORTUGUESE, "portuguese" },
	{ LanguageType::ARABIC, "arabic" },
	{ LanguageType::NORWEGIAN, "norwegian" },
	{ LanguageType::POLISH, "polish" },
	{ LanguageType::TURKISH, "turkish" },
	{ LanguageType::UKRAINIAN, "ukrainian" },
	{ LanguageType::ROMANIAN, "romanian" },
	{ LanguageType::BULGARIAN, "bulgarian" },
};

Language::Language()
{
	Application::getInstance()->getCurrentLanguage();
}

Language::~Language()
{

}

void Language::set_language(LanguageType type)
{
	if (type != LanguageType::CHINESE)
	{
		type = LanguageType::ENGLISH;
	}

	auto result = g_language_map.find(type);
	assert(result != g_language_map.end());
	if (result != g_language_map.end())
	{
		rapidjson::Document doc;
		Data data = FileUtils::getInstance()->getDataFromFile("config/language.json");
		doc.Parse<0>(std::string((const char *)data.getBytes(), data.getSize()).c_str());
		if (!doc.HasParseError() && doc.HasMember(result->second))
		{
			corresponding_table_.clear();
			const rapidjson::Value &json_object = doc[result->second];
			for (auto itr = json_object.MemberBegin(); itr != json_object.MemberEnd(); ++itr)
			{
				corresponding_table_.insert(std::make_pair(itr->name.GetString(), itr->value.GetString()));
			}
		}
	}
}

const std::string& lang(const std::string &text)
{
	auto result = Language::instance()->corresponding_table_.find(text);
	if (result != Language::instance()->corresponding_table_.end())
	{
		return result->second;
	}
	return std::string();
}
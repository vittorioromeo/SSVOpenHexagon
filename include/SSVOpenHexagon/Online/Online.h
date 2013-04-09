// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_SERVER
#define HG_SERVER

#include <jsoncpp/json.h>
#include <SSVStart/SSVStart.h>

namespace hg
{
	namespace Online
	{
		void startCheckUpdates();
		void startSendScore(const std::string& mName, const std::string& mValidator, float mDifficulty, float mScore);
		void startGetScores(std::string& mTargetScores, std::string& mTargetPlayerScore, const std::string& mName, const std::string& mValidator, float mDifficulty);

		void cleanUp();
		void terminateAll();

		std::string getValidator(const std::string& mPackPath, const std::string& mLevelId, const std::string& mLevelRootPath, const std::string& mStyleRootPath, const std::string& mLuaScriptPath);

		float getServerVersion();
		std::string getServerMessage();
		Json::Value getScores(const std::string& mValidator);
		std::string getMD5Hash(const std::string& mString);
		std::string getUrlEncoded(const std::string& mString);
		std::string getControlStripped(const std::string& mString);
	}
}

#endif

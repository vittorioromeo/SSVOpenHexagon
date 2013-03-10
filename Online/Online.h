// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_SERVER
#define HG_SERVER

namespace hg
{
	namespace Online
	{	
		void startCheckUpdates();
		void startCheckScores();
		void startSendScore(const std::string& mName, const std::string& mValidator, float mScore);

		void cleanUp();
		void terminateAll();

		float getServerVersion();
		Json::Value getScores(const std::string& mValidator);
		std::string getValidator(const std::string& mLevelId, const std::string& mJsonRootPath, const std::string& mLuaScriptPath, float mDifficultyMultiplier);
	}
}

#endif

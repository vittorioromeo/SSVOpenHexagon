#ifndef HG_SERVER
#define HG_SERVER

namespace hg
{
	namespace Online
	{	
		void startCheckUpdates();
		void startCheckScores();
		void startSendScore(const std::string& mName, const std::string& mValidator, float mScore);

		void freeMemory();
		void cleanUp();

		float getServerVersion();
		Json::Value getScores(const std::string& mValidator);
		std::string getValidator(const std::string& mLevelId, const std::string& mJsonRootPath, const std::string& mLuaScriptPath, float mDifficultyMultiplier);
		std::string getStripped(const std::string& mString);
		std::string getCompressed(const std::string& mString);
	}
}

#endif

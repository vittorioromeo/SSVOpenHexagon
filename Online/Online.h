#ifndef HG_SERVER
#define HG_SERVER

namespace hg
{
	namespace Online
	{
		void checkUpdates();
		void sendScore(const std::string& mProfileName, const std::string& mLevelValidator, float mScore);
		std::vector<std::pair<std::string, float>>& getScores(const std::string& mLevelValidator);
		void getLeaderboard(std::string& mLeaderboard, const std::string& mLevelValidator);

		void cleanUp();

		void setUpdatesChecked(bool mUpdatesChecked);
		bool getUpdatesChecked();
		void setServerVersion(float mServerVersion);
		float getServerVersion();

		std::string getStripped(const std::string& mString);
		std::string getCompressed(const std::string& mString);
	}
}

#endif

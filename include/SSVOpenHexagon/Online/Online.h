// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE
#define HG_ONLINE

#include "SSVOpenHexagon/Global/Common.h"
#include "SSVOpenHexagon/Online/Compression.h"

namespace hg
{
	class HGAssets;

	namespace Online
	{
		struct UserStats;

		class ValidatorDB
		{
			private:
				std::string nullString{"NULL"};
				std::unordered_map<std::string, std::string> validators;

			public:
				void addValidator(const std::string& mLevelId, const std::string& mValidator) { validators[mLevelId] = mValidator; }
				const std::string& getValidator(const std::string& mLevelId) const { return validators.count(mLevelId) == 0 ? nullString : validators.at(mLevelId); }
		};

		enum class ConnectStat{Disconnected, Connecting, Connected};
		enum class LoginStat{Unlogged, Logging, Logged, NewUserReg};

		void initalizeValidators(HGAssets& mAssets);
		void initializeClient();

		void tryConnectToServer();
		void tryLogin(const std::string& mUsername, const std::string& mPassword);

		void trySendScore(const std::string& mLevelId, float mDiffMult, float mScore);
		void tryRequestLeaderboard(const std::string& mLevelId, float mDiffMult);
		void trySendDeath();
		void trySendMinutePlayed();
		void trySendRestart();
		void trySendInitialRequests();
		void trySendAddFriend(const std::string& mFriendName);
		void trySendClearFriends();
		void tryRequestFriendsScores(const std::string& mLevelId, float mDiffMult);
		void trySendUserEmail(const std::string& mEmail);

		void requestLeaderboardIfNeeded(const std::string& mLevelId, float mDiffMult);

		ConnectStat getConnectionStatus();
		LoginStat getLoginStatus();

		void logout();

		ValidatorDB& getValidators();

		std::string getCurrentUsername();

		const UserStats& getUserStats();

		void invalidateCurrentLeaderboard();
		void invalidateCurrentFriendsScores();
		const std::string& getCurrentLeaderboard();
		const ssvuj::Obj& getCurrentFriendScores();
		void setForceLeaderboardRefresh(bool mValue);

		float getServerVersion();
		std::string getServerMessage();
		std::string getMD5Hash(const std::string& mStr);
		inline std::string getControlStripped(const std::string& mStr)	{ std::string result; for(const auto& c : mStr) if(!std::iscntrl(c)) result += c; return result; }
		inline std::string getUrlEncoded(const std::string& mStr) 		{ std::string result; for(const auto& c : mStr) if(std::isalnum(c)) result += c; return getControlStripped(result); }

		bool getNewUserReg();

		// Client to server
		enum FromClient : unsigned int
		{
			Ping = 0,
			Login,
			RequestInfo,
			SendScore,
			RequestLeaderboard,
			RequestUserStats,
			US_Death,
			US_MinutePlayed,
			US_Restart,
			US_AddFriend,
			US_ClearFriends,
			RequestFriendsScores,
			Logout,
			NUR_Email
		};

		// Server to client
		enum FromServer : unsigned int
		{
			LoginResponseValid = 0,
			LoginResponseInvalid,
			RequestInfoResponse,
			SendLeaderboard,
			SendScoreResponseValid,
			SendScoreResponseInvalid,
			SendLeaderboardFailed,
			SendUserStats,
			SendUserStatsFailed,
			SendFriendsScores,
			SendLogoutValid,
			NUR_EmailValid
		};
	}
}





#endif

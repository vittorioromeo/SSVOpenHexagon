// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE
#define HG_ONLINE

#include <string>
#include <future>
#include <unordered_map>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>
#include <SFML/Network.hpp>
#include "SSVOpenHexagon/Online/Compression.h"

namespace hg
{
	class HGAssets;

	namespace Online
	{
		class UserStats;

		class ValidatorDB
		{
			private:
				std::string nullString{"NULL"};
				std::unordered_map<std::string, std::string> validators;

			public:
				void addValidator(const std::string& mLevelId, const std::string& mValidator) { validators[mLevelId] = mValidator; }
				const std::string& getValidator(const std::string& mLevelId) const { return validators.count(mLevelId) == 0 ? nullString : validators.at(mLevelId); }
		};

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

		bool isConnected();
		bool isLoggedIn();
		bool isLoginTimedOut();

		void logout();

		ValidatorDB& getValidators();

		std::string getCurrentUsername();

		const UserStats& getUserStats();

		void invalidateCurrentLeaderboard();
		void invalidateCurrentFriendsScores();
		const std::string& getCurrentLeaderboard();
		const ssvuj::Value& getCurrentFriendScores();

		float getServerVersion();
		std::string getServerMessage();
		std::string getMD5Hash(const std::string& mString);
		inline std::string getUrlEncoded(const std::string& mString) 		{ std::string result; for(const auto& c : mString) if(std::isalnum(c)) result += c; return result; }
		inline std::string getControlStripped(const std::string& mString)	{ std::string result; for(const auto& c : mString) if(!std::iscntrl(c)) result += c; return result; }

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
			Logout
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
			SendLogoutValid
		};
	}
}





#endif

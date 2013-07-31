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
		void trySendScore(const std::string& mUsername, const std::string& mLevelId, float mDiffMult, float mScore);
		void tryRequestLeaderboard(const std::string& mUsername, const std::string& mLevelId, float mDiffMult);

		bool isConnected();
		bool isLoggedIn();
		bool isLoginTimedOut();

		ValidatorDB& getValidators();

		const std::string& getCurrentUsername();

		void invalidateCurrentLeaderboard();
		const std::string& getCurrentLeaderboard();

		float getServerVersion();
		std::string getServerMessage();
		std::string getMD5Hash(const std::string& mString);
		inline std::string getUrlEncoded(const std::string& mString) 		{ std::string result; for(const auto& c : mString) if(std::isalnum(c)) result += c; return result; }
		inline std::string getControlStripped(const std::string& mString)	{ std::string result; for(const auto& c : mString) if(!std::iscntrl(c)) result += c; return result; }


		// Client to server
		enum FromClient : unsigned int
		{
			Ping = 0,
			Login = 1,
			RequestInfo = 2,
			SendScore = 3,
			RequestLeaderboard = 4
		};

		// Server to client
		enum FromServer : unsigned int
		{
			LoginResponseValid = 0,
			LoginResponseInvalid = 1,
			RequestInfoResponse = 2,
			SendLeaderboard = 3,
			SendScoreResponseValid = 4,
			SendScoreResponseInvalid = 5,
			SendLeaderboardFailed = 6
		};
	}
}





#endif

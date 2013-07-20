// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE
#define HG_ONLINE

#include <future>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>
#include <SFML/Network.hpp>

namespace hg
{
	namespace Online
	{
		void initializeServer();
		void initializeClient();

		void tryConnectToServer();
		bool isConnected();

		void tryLogin(const std::string& mUsername, const std::string& mPassword);
		bool isLoggedIn();

		//void startCheckUpdates();
		//void startSendScore(const std::string& mName, const std::string& mValidator, float mDifficulty, float mScore);
		//void startGetScores(std::string& mTargetScores, std::string& mTargetPlayerScore, const std::string& mName, std::vector<std::string>& mTarget, const std::vector<std::string>& mNames, const std::string& mValidator, float mDifficulty);
		//void startGetFriendsScores(std::vector<std::string>& mTarget, const std::vector<std::string>& mNames, const std::string& mValidator, float mDifficulty);

		//void cleanUp();
		//void terminateAll();

		//std::string getValidator(const std::string& mPackPath, const std::string& mLevelId, const std::string& mLevelRootPath, const std::string& mStyleRootPath, const std::string& mLuaScriptPath);

		float getServerVersion();
		std::string getServerMessage();
		std::string getMD5Hash(const std::string& mString);
		inline std::string getUrlEncoded(const std::string& mString) 		{ std::string result; for(const auto& c : mString) if(std::isalnum(c)) result += c; return result; }
		inline std::string getControlStripped(const std::string& mString)	{ std::string result; for(const auto& c : mString) if(!std::iscntrl(c)) result += c; return result; }

		bool isOverloaded();
		bool isFree();

		enum class LogMode { Quiet, Verbose };

		enum ClientPackets : unsigned int
		{
			Ping = 0,
			Data = 1,
			Login = 2
		};

		enum ServerPackets : unsigned int
		{
			LoginResponse = 0
		};

		template<LogMode TLM = LogMode::Quiet> std::future<bool> asyncTry(std::function<bool()> mFunc, const std::chrono::duration<int, std::milli>& mDuration = std::chrono::milliseconds(1500), int mTimes = 5)
		{
			auto result(std::async(std::launch::async, [=]
			{
				for(int i{0}; i < mTimes; ++i)
				{
					if(mFunc()) return true;

					if(TLM == LogMode::Verbose) ssvu::log("Error - retrying (" + ssvu::toStr(i + 1) + "/" + ssvu::toStr(mTimes) + ")", "asyncTry");
					std::this_thread::sleep_for(mDuration);
				}

				return false;
			}));

			return result;
		}

		inline sf::Packet buildPingPacket()							{ sf::Packet result; result << ClientPackets::Ping; return result; }
		inline sf::Packet buildHelloPacket()						{ sf::Packet result; result << ClientPackets::Data << "hello bro!"; return result; }
	}
}

#endif

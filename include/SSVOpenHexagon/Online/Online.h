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
		using Listener = sf::TcpListener;
		using Packet = sf::Packet;
		using Socket = sf::TcpSocket;
		using IpAddress = sf::IpAddress;

		void initializeServer();
		void initializeClient();

		void tryConnectToServer();
		bool isConnected();

		void tryLogin(const std::string& mUsername, const std::string& mPassword);
		bool isLoggedIn();

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

		template<int TTimes = 5, LogMode TLM = LogMode::Quiet> std::future<bool> retry(std::function<bool()> mFunc, const std::chrono::duration<int, std::milli>& mDuration = std::chrono::milliseconds(1500))
		{
			auto result(std::async(std::launch::async, [=]
			{
				for(int i{0}; i < TTimes; ++i)
				{
					if(mFunc()) return true;

					if(TLM == LogMode::Verbose) ssvu::log("Error - retrying (" + ssvu::toStr(i + 1) + "/" + ssvu::toStr(TTimes) + ")", "asyncTry");
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

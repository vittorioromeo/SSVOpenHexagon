// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <functional>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Client.h"
#include "SSVOpenHexagon/Online/Server.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Online/Definitions.h"
#include "SSVOpenHexagon/Utils/Utils.h"

#include "SSVOpenHexagon/Global/Assets.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvu::Encryption;
using namespace ssvuj;
using namespace ssvu::FileSystem;

namespace hg
{
	namespace Online
	{
		const IpAddress hostIp{"209.236.124.147"};
		const unsigned short hostPort{27272};

		bool connected{false}, connecting{false};
		bool loggedIn{false}; string currentUsername;
		bool loginTimedOut{false};

		float serverVersion{-1};
		string serverMessage{""};

		ValidatorDB validators;

		PacketHandler clientPHandler;
		Uptr<Client> client;

		string currentLeaderboard{"NULL"};

		ValidatorDB& getValidators() { return validators; }


		void initializeClient()
		{
			clientPHandler[FromServer::LoginResponseValid] = [](ManagedSocket& mMS, sf::Packet&)	{ lo << lt("PacketHandler") << "Successfully logged in!" << endl; loggedIn = true; mMS.send(buildPacket<FromClient::RequestInfo>()); };
			clientPHandler[FromServer::LoginResponseInvalid] = [](ManagedSocket&, sf::Packet&)		{ loggedIn = false; loginTimedOut = true; lo << lt("PacketHandler") << "Login invalid!" << endl; };
			clientPHandler[FromServer::RequestInfoResponse] = [](ManagedSocket&, sf::Packet& mP)	{ ssvuj::Value r{getDecompressedPacket(mP)}; serverVersion = ssvuj::as<float>(r, 0); serverMessage = ssvuj::as<string>(r, 1); };
			clientPHandler[FromServer::SendLeaderboard] = [](ManagedSocket&, sf::Packet& mP)		{ currentLeaderboard = ssvuj::as<string>(getDecompressedPacket(mP), 0); };
			clientPHandler[FromServer::SendLeaderboardFailed] = [](ManagedSocket&, sf::Packet&)		{ currentLeaderboard = "NULL"; };
			clientPHandler[FromServer::SendScoreResponseValid] = [](ManagedSocket&, sf::Packet&)	{ lo << lt("PacketHandler") << "Server successfully accepted score"; };
			clientPHandler[FromServer::SendScoreResponseInvalid] = [](ManagedSocket&, sf::Packet&)	{ lo << lt("PacketHandler") << "Server refused score - level data doesn't match server's"; };

			client = Uptr<Client>(new Client(clientPHandler));

			thread([]
			{
				while(true)
				{
					if(connected) { client->send(buildPacket<FromClient::Ping>()); }
					if(!client->getManagedSocket().isBusy()) connected = false;
					this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}).detach();
		}

		void tryConnectToServer()
		{
			if(connected || connecting) { lo << lt("hg::Online::connectToServer") << "Already connected" << endl; return; }
			connecting = true;

			lo << lt("hg::Online::connectToServer") << "Connecting to server..." << endl;

			thread([]
			{
				if(client->connect("127.0.0.1", 54000))
				{
					lo << lt("hg::Online::connectToServer") << "Connected to server!" << endl;
					connecting = false; connected = true; return;
				}

				lo << lt("hg::Online::connectToServer") << "Failed to connect" << endl;
				connected = false; connecting = false;
			}).detach();
		}

		void tryLogin(const string& mUsername, const string& mPassword)
		{
			lo << lt("hg::Online::tryLogin") << "Logging in..." << endl;

			thread([=]
			{
				loginTimedOut = false;
				this_thread::sleep_for(std::chrono::milliseconds(100));
				if(!retry([&]{ return connected; }).get()) { lo << lt("hg::Online::tryLogin") << "Client not connected - aborting" << endl; loginTimedOut = true; return; }
				client->send(buildCompressedPacket<FromClient::Login>(mUsername, mPassword));
				currentUsername = mUsername;
			}).detach();
		}


		void trySendScore(const string& mUsername, const string& mLevelId, float mDiffMult, float mScore)
		{
			lo << lt("hg::Online::trySendScore") << "Sending score..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(100));
				if(!connected) { lo << lt("hg::Online::trySendScore") << "Client not connected - aborting" << endl; return; }
				client->send(buildCompressedPacket<FromClient::SendScore>(mUsername, mLevelId, validators.getValidator(mLevelId), mDiffMult, mScore));
			}).detach();
		}

		void tryRequestLeaderboard(const std::string& mUsername, const string& mLevelId, float mDiffMult)
		{
			lo << lt("hg::Online::tryRequestLeaderboard") << "Requesting scores..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(100));
				if(!connected) { lo << lt("hg::Online::tryRequestLeaderboard") << "Client not connected - aborting" << endl; return; }
				client->send(buildCompressedPacket<FromClient::RequestLeaderboard>(mUsername, mLevelId, validators.getValidator(mLevelId), mDiffMult));
			}).detach();
		}

		bool isConnected()	{ return connected; }
		bool isLoggedIn()	{ return loggedIn; }
		bool isLoginTimedOut()	{ return loginTimedOut; }
		const string& getCurrentUsername() { if(!loggedIn) throw; return currentUsername; }

		void invalidateCurrentLeaderboard() { currentLeaderboard = "NULL"; }
		const string& getCurrentLeaderboard() { return currentLeaderboard; }

		string getValidator(const string& mPackPath, const string& mLevelId, const string& mLevelRootString, const string& mStyleRootPath, const string& mLuaScriptPath)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt{""};
			toEncrypt.append(mLevelId);
			toEncrypt.append(mLevelRootString);
			toEncrypt.append(getFileContents(mStyleRootPath));
			toEncrypt.append(luaScriptContents);

			for(const auto& lsn : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + lsn};
				string contents{getFileContents(path)};
				toEncrypt.append(contents);
			}

			toEncrypt = getControlStripped(toEncrypt);

			return getUrlEncoded(mLevelId) + getMD5Hash(toEncrypt + HG_SKEY1 + HG_SKEY2 + HG_SKEY3);
		}

		float getServerVersion()					{ return serverVersion; }
		string getServerMessage()					{ return serverMessage; }
		string getMD5Hash(const string& mString)	{ return encrypt<Encryption::Type::MD5>(mString); }

		void initalizeValidators(HGAssets& mAssets)
		{
			lo << lt("hg::Online::initalizeValidators") << "Initializing validators..." << endl;

			for(const auto& p : mAssets.getLevelDatas())
			{
				const auto& l(p.second);
				const auto& validator(getValidator(l->packPath, l->id, l->getRootString(), mAssets.getStyleData(l->styleId).getRootPath(), l->luaScriptPath));
				validators.addValidator(p.first, validator);

				//lo << lt("hg::Online::initalizeValidators") << "Added (" << p.first << "): " << validator << endl;
			}
		}
	}
}


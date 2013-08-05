// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Client.h"
#include "SSVOpenHexagon/Online/Server.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Online/Definitions.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Online/OHServer.h"
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

		ConnectionStatus connectionStatus{ConnectionStatus::Disconnected};
		LoginStatus loginStatus{LoginStatus::Unlogged};

		float serverVersion{-1};
		string serverMessage;

		ValidatorDB validators;

		PacketHandler clientPHandler;
		Uptr<Client> client;

		string currentUsername{"NULL"};
		string currentLeaderboard{"NULL"};
		string currentUserStatsStr{"NULL"};
		UserStats currentUserStats;
		ssvuj::Value currentFriendScores;

		const ssvuj::Value& getCurrentFriendScores() { return currentFriendScores; }

		const UserStats& getUserStats() { return currentUserStats; }

		ValidatorDB& getValidators() { return validators; }

		void refreshUserStats()
		{
			ssvuj::Value root{getRootFromString(currentUserStatsStr)};
			currentUserStats = ssvuj::as<UserStats>(root);
		}

		void initializeClient()
		{
			clientPHandler[FromServer::LoginResponseValid] = [](ManagedSocket&, sf::Packet&)		{ lo << lt("PacketHandler") << "Successfully logged in!" << endl; loginStatus = LoginStatus::Logged; trySendInitialRequests(); };
			clientPHandler[FromServer::LoginResponseInvalid] = [](ManagedSocket&, sf::Packet&)		{ loginStatus = LoginStatus::TimedOut; lo << lt("PacketHandler") << "Login invalid!" << endl; };
			clientPHandler[FromServer::RequestInfoResponse] = [](ManagedSocket&, sf::Packet& mP)	{ ssvuj::Value r{getDecompressedPacket(mP)}; serverVersion = ssvuj::as<float>(r, 0); serverMessage = ssvuj::as<string>(r, 1); };
			clientPHandler[FromServer::SendLeaderboard] = [](ManagedSocket&, sf::Packet& mP)		{ currentLeaderboard = ssvuj::as<string>(getDecompressedPacket(mP), 0); };
			clientPHandler[FromServer::SendLeaderboardFailed] = [](ManagedSocket&, sf::Packet&)		{ currentLeaderboard = "NULL"; lo << lt("PacketHandler") << "Server failed sending leaderboard"; };
			clientPHandler[FromServer::SendScoreResponseValid] = [](ManagedSocket&, sf::Packet&)	{ lo << lt("PacketHandler") << "Server successfully accepted score"; };
			clientPHandler[FromServer::SendScoreResponseInvalid] = [](ManagedSocket&, sf::Packet&)	{ lo << lt("PacketHandler") << "Server refused score - level data doesn't match server's"; };
			clientPHandler[FromServer::SendUserStats] = [](ManagedSocket&, sf::Packet& mP)			{ currentUserStatsStr = ssvuj::as<string>(getDecompressedPacket(mP), 0); refreshUserStats(); };
			clientPHandler[FromServer::SendUserStatsFailed] = [](ManagedSocket&, sf::Packet&)		{ currentUserStatsStr = "NULL"; lo << lt("PacketHandler") << "Server failed sending user stats"; };
			clientPHandler[FromServer::SendFriendsScores] = [](ManagedSocket&, sf::Packet& mP)		{ currentFriendScores = ssvuj::getRootFromString(ssvuj::as<string>(getDecompressedPacket(mP), 0));};
			clientPHandler[FromServer::SendLogoutValid] = [](ManagedSocket&, sf::Packet&)			{ loginStatus = LoginStatus::Unlogged; };

			client = Uptr<Client>(new Client(clientPHandler));

			thread([]
			{
				while(true)
				{
					if(connectionStatus == ConnectionStatus::Connected) client->send(buildCPacket<FromClient::Ping>());
					if(!client->getManagedSocket().isBusy())
					{
						connectionStatus = ConnectionStatus::Disconnected;
						loginStatus = LoginStatus::Unlogged;
					}
					this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}).detach();
		}

		void tryConnectToServer()
		{
			if(connectionStatus == ConnectionStatus::Connecting)	{ lo << lt("hg::Online::connectToServer") << "Already connecting" << endl; return; }
			if(connectionStatus == ConnectionStatus::Connected)		{ lo << lt("hg::Online::connectToServer") << "Already connected" << endl; return; }

			lo << lt("hg::Online::connectToServer") << "Connecting to server..." << endl;
			connectionStatus = ConnectionStatus::Connecting;

			thread([]
			{
				if(client->connect("127.0.0.1", 54000))
				{
					lo << lt("hg::Online::connectToServer") << "Connected to server!" << endl;
					connectionStatus = ConnectionStatus::Connected; return;
				}

				lo << lt("hg::Online::connectToServer") << "Failed to connect" << endl;
				connectionStatus = ConnectionStatus::Disconnected;
			}).detach();
		}

		void tryLogin(const string& mUsername, const string& mPassword)
		{
			if(loginStatus != LoginStatus::Unlogged) { logout(); return; }

			lo << lt("hg::Online::tryLogin") << "Logging in..." << endl;
			loginStatus = LoginStatus::Logging;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(80));
				if(!retry([&]{ return connectionStatus == ConnectionStatus::Connected; }).get()) { lo << lt("hg::Online::tryLogin") << "Client not connected - aborting" << endl; loginStatus = LoginStatus::TimedOut; return; }
				client->send(buildCPacket<FromClient::Login>(mUsername, mPassword));
				currentUsername = mUsername;
			}).detach();
		}

		bool canSendPacket() { return connectionStatus == ConnectionStatus::Connected && loginStatus == LoginStatus::Logged && currentUsername != "NULL"; }

		template<typename T> void trySendFunc(T&& mFunc)
		{
			if(!canSendPacket()) { lo << lt("hg::Online::trySendFunc") << "Can't send data to server: not connected / not logged in" << endl; return; }
			lo << lt("hg::Online::trySendFunc") << "Sending data to server..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(80));
				if(!canSendPacket()) { lo << lt("hg::Online::trySendFunc") << "Client not connected - aborting" << endl; return; }
				mFunc();
			}).detach();
		}

		template<unsigned int TType, typename... TArgs> void trySendPacket(TArgs&&... mArgs)
		{

		}

		void trySendScore(const string& mLevelId, float mDiffMult, float mScore)	{ trySendFunc([=]{ client->send(buildCPacket<FromClient::SendScore>(currentUsername, mLevelId, validators.getValidator(mLevelId), mDiffMult, mScore)); }); }
		void tryRequestLeaderboard(const string& mLevelId, float mDiffMult)			{ trySendFunc([=]{ client->send(buildCPacket<FromClient::RequestLeaderboard>(currentUsername, mLevelId, validators.getValidator(mLevelId), mDiffMult)); }); }
		void trySendDeath()															{ trySendFunc([=]{ client->send(buildCPacket<FromClient::US_Death>(currentUsername)); }); }
		void trySendMinutePlayed()													{ trySendFunc([=]{ client->send(buildCPacket<FromClient::US_MinutePlayed>(currentUsername)); }); }
		void trySendRestart()														{ trySendFunc([=]{ client->send(buildCPacket<FromClient::US_Restart>(currentUsername)); }); }
		void trySendInitialRequests()												{ trySendFunc([=]{ client->send(buildCPacket<FromClient::RequestInfo>()); client->send(buildCPacket<FromClient::RequestUserStats>(currentUsername));}); }
		void trySendAddFriend(const string& mFriendName)							{ trySendFunc([=]{ client->send(buildCPacket<FromClient::US_AddFriend>(currentUsername, mFriendName)); trySendInitialRequests(); }); }
		void trySendClearFriends()													{ trySendFunc([=]{ client->send(buildCPacket<FromClient::US_ClearFriends>(currentUsername)); trySendInitialRequests(); }); }
		void tryRequestFriendsScores(const string& mLevelId, float mDiffMult)		{ trySendFunc([=]{ client->send(buildCPacket<FromClient::RequestFriendsScores>(currentUsername, mLevelId, mDiffMult)); }); }

		ConnectionStatus getConnectionStatus()	{ return connectionStatus; }
		LoginStatus getLoginStatus()			{ return loginStatus; }
		string getCurrentUsername()				{ return loginStatus == LoginStatus::Logged ? currentUsername : "NULL"; }

		void logout() { trySendFunc([=]{ client->send(buildCPacket<FromClient::Logout>(currentUsername)); }); }

		void invalidateCurrentLeaderboard() { currentLeaderboard = "NULL"; }
		void invalidateCurrentFriendsScores() { currentFriendScores = ssvuj::Value{}; }
		const string& getCurrentLeaderboard() { return currentLeaderboard; }

		string getValidator(const string& mPackPath, const string& mLevelId, const string& mLevelRootString, const string& mStyleRootPath, const string& mLuaScriptPath)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt;
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


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
		const unsigned short hostPort{27273};

		ConnectStat connectionStatus{ConnectStat::Disconnected};
		LoginStat loginStatus{LoginStat::Unlogged};

		float serverVersion{-1};
		string serverMessage;

		ValidatorDB validators;

		PacketHandler<Client> clientPHandler;
		Uptr<Client> client;

		bool gettingLeaderboard{false};
		string lastLeaderboardId;
		float lastLeaderboardDM;

		bool newUserReg{false};

		string currentUsername{"NULL"};
		string currentLeaderboard{"NULL"};
		string currentUserStatsStr{"NULL"};
		UserStats currentUserStats;
		ssvuj::Obj currentFriendScores;

		void refreshUserStats()
		{
			ssvuj::Obj root{readFromString(currentUserStatsStr)};
			currentUserStats = ssvuj::as<UserStats>(root);
		}

		void initializeClient()
		{
			clientPHandler[FromServer::LoginResponseValid] = [](Client&, sf::Packet& mP)
			{
				lo << lt("PacketHandler") << "Successfully logged in!" << endl;
				loginStatus = LoginStat::Logged;
				newUserReg = ssvuj::as<bool>(getDecompressedPacket(mP), 0);
				trySendInitialRequests();
			};
			clientPHandler[FromServer::LoginResponseInvalid] = [](Client&, sf::Packet&)		{ loginStatus = LoginStat::Unlogged; lo << lt("PacketHandler") << "Login invalid!" << endl; };
			clientPHandler[FromServer::RequestInfoResponse] = [](Client&, sf::Packet& mP)	{ ssvuj::Obj r{getDecompressedPacket(mP)}; serverVersion = ssvuj::as<float>(r, 0); serverMessage = ssvuj::as<string>(r, 1); };
			clientPHandler[FromServer::SendLeaderboard] = [](Client&, sf::Packet& mP)		{ currentLeaderboard = ssvuj::as<string>(getDecompressedPacket(mP), 0); gettingLeaderboard = false; };
			clientPHandler[FromServer::SendLeaderboardFailed] = [](Client&, sf::Packet&)	{ currentLeaderboard = "NULL"; lo << lt("PacketHandler") << "Server failed sending leaderboard" << endl; gettingLeaderboard = false; };
			clientPHandler[FromServer::SendScoreResponseValid] = [](Client&, sf::Packet&)	{ lo << lt("PacketHandler") << "Server successfully accepted score" << endl; };
			clientPHandler[FromServer::SendScoreResponseInvalid] = [](Client&, sf::Packet&)	{ lo << lt("PacketHandler") << "Server refused score" << endl; };
			clientPHandler[FromServer::SendUserStats] = [](Client&, sf::Packet& mP)			{ currentUserStatsStr = ssvuj::as<string>(getDecompressedPacket(mP), 0); refreshUserStats(); };
			clientPHandler[FromServer::SendUserStatsFailed] = [](Client&, sf::Packet&)		{ currentUserStatsStr = "NULL"; lo << lt("PacketHandler") << "Server failed sending user stats" << endl; };
			clientPHandler[FromServer::SendFriendsScores] = [](Client&, sf::Packet& mP)		{ currentFriendScores = ssvuj::readFromString(ssvuj::as<string>(getDecompressedPacket(mP), 0));};
			clientPHandler[FromServer::SendLogoutValid] = [](Client&, sf::Packet&)			{ loginStatus = LoginStat::Unlogged; };
			clientPHandler[FromServer::NUR_EmailValid] = [](Client&, sf::Packet&)			{ newUserReg = false; };

			client = Uptr<Client>(new Client(clientPHandler));

			thread([]
			{
				while(true)
				{
					if(connectionStatus == ConnectStat::Connected)
					{
						client->send(buildCPacket<FromClient::Ping>());
						if(!client->isBusy())
						{
							connectionStatus = ConnectStat::Disconnected;
							loginStatus = LoginStat::Unlogged;
						}
					}
					this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}).detach();
		}

		bool canSendPacket() { return connectionStatus == ConnectStat::Connected && loginStatus == LoginStat::Logged && currentUsername != "NULL"; }

		template<typename T> void trySendFunc(T&& mFunc)
		{
			if(!canSendPacket()) { lo << lt("hg::Online::trySendFunc") << "Can't send data to server: not connected / not logged in" << endl; return; }
			//lo << lt("hg::Online::trySendFunc") << "Sending data to server..." << endl;

			thread([=]
			{
				if(!canSendPacket()) { lo << lt("hg::Online::trySendFunc") << "Client not connected - aborting" << endl; return; }
				mFunc();
			}).detach();
		}

		template<unsigned int TType, typename... TArgs> void trySendPacket(TArgs&&... mArgs)
		{
			const auto& packet(buildCPacket<TType>(mArgs...));
			trySendFunc([=]{ client->send(packet); });
		}

		void tryConnectToServer()
		{
			if(connectionStatus == ConnectStat::Connecting)	{ lo << lt("hg::Online::connectToServer") << "Already connecting" << endl; return; }
			if(connectionStatus == ConnectStat::Connected)	{ lo << lt("hg::Online::connectToServer") << "Already connected" << endl; return; }

			lo << lt("hg::Online::connectToServer") << "Connecting to server..." << endl;
			client->disconnect();
			connectionStatus = ConnectStat::Connecting;

			thread([]
			{
				//if(client->connect("127.0.0.1", 54000))
				if(client->connect(hostIp, hostPort))
				{
					lo << lt("hg::Online::connectToServer") << "Connected to server!" << endl;
					connectionStatus = ConnectStat::Connected; return;
				}

				lo << lt("hg::Online::connectToServer") << "Failed to connect" << endl;
				connectionStatus = ConnectStat::Disconnected;
				client->disconnect();
			}).detach();
		}

		void tryLogin(const string& mUsername, const string& mPassword)
		{
			if(loginStatus != LoginStat::Unlogged) { logout(); return; }
			if(connectionStatus != ConnectStat::Connected) { lo << lt("hg::Online::tryLogin") << "Client not connected - aborting" << endl; loginStatus = LoginStat::Unlogged; return; }

			lo << lt("hg::Online::tryLogin") << "Logging in..." << endl;
			loginStatus = LoginStat::Logging;

			thread([=]
			{
				client->send(buildCPacket<FromClient::Login>(mUsername, mPassword));
				currentUsername = mUsername;
			}).detach();
		}





		void trySendScore(const string& mLevelId, float mDiffMult, float mScore)	{ trySendPacket<FromClient::SendScore>(currentUsername, mLevelId, validators.getValidator(mLevelId), mDiffMult, mScore); }
		void tryRequestLeaderboard(const string& mLevelId, float mDiffMult)			{ trySendPacket<FromClient::RequestLeaderboard>(currentUsername, mLevelId, validators.getValidator(mLevelId), mDiffMult); }
		void trySendDeath()															{ trySendPacket<FromClient::US_Death>(currentUsername); }
		void trySendMinutePlayed()													{ trySendPacket<FromClient::US_MinutePlayed>(currentUsername); }
		void trySendRestart()														{ trySendPacket<FromClient::US_Restart>(currentUsername); }
		void trySendInitialRequests()												{ trySendPacket<FromClient::RequestInfo>(); trySendPacket<FromClient::RequestUserStats>(currentUsername); }
		void trySendAddFriend(const string& mFriendName)							{ trySendPacket<FromClient::US_AddFriend>(currentUsername, mFriendName); trySendInitialRequests(); }
		void trySendClearFriends()													{ trySendPacket<FromClient::US_ClearFriends>(currentUsername); trySendInitialRequests(); }
		void tryRequestFriendsScores(const string& mLevelId, float mDiffMult)		{ trySendPacket<FromClient::RequestFriendsScores>(currentUsername, mLevelId, mDiffMult); }
		void trySendUserEmail(const string& mEmail)									{ trySendPacket<FromClient::NUR_Email>(currentUsername, mEmail); }
		void logout()																{ trySendPacket<FromClient::Logout>(currentUsername); }

		void requestLeaderboardIfNeeded(const string& mLevelId, float mDiffMult)
		{
			if(gettingLeaderboard || (lastLeaderboardId == mLevelId && lastLeaderboardDM == mDiffMult)) return;
			invalidateCurrentLeaderboard();
			invalidateCurrentFriendsScores();
			gettingLeaderboard = true;
			lastLeaderboardId = mLevelId;
			lastLeaderboardDM = mDiffMult;
			tryRequestLeaderboard(mLevelId, mDiffMult);
			tryRequestFriendsScores(mLevelId, mDiffMult);
			trySendPacket<FromClient::RequestUserStats>(currentUsername);
		}

		ConnectStat getConnectionStatus()			{ return connectionStatus; }
		LoginStat getLoginStatus()					{ return loginStatus; }
		string getCurrentUsername()					{ return loginStatus == LoginStat::Logged ? currentUsername : "NULL"; }
		const ssvuj::Obj& getCurrentFriendScores()	{ return currentFriendScores; }
		const UserStats& getUserStats()				{ return currentUserStats; }
		ValidatorDB& getValidators()				{ return validators; }
		bool getNewUserReg()						{ return newUserReg; }
		void invalidateCurrentLeaderboard()			{ currentLeaderboard = "NULL"; }
		void invalidateCurrentFriendsScores()		{ currentFriendScores = ssvuj::Obj{}; }
		const string& getCurrentLeaderboard()		{ return currentLeaderboard; }
		float getServerVersion()					{ return serverVersion; }
		string getServerMessage()					{ return serverMessage; }

		string getValidator(const string& mPackPath, const string& mLevelId, const string& mLevelRootString, const string& mStyleRootPath, const string& mLuaScriptPath)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};
			std::set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt;
			toEncrypt += mLevelId;
			toEncrypt += getControlStripped(mLevelRootString);
			toEncrypt += getFileContents(mStyleRootPath);
			toEncrypt += luaScriptContents;
			for(const auto& lsn : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + lsn};
				toEncrypt += getFileContents(path);
			}

			toEncrypt = getControlStripped(toEncrypt);
			return getUrlEncoded(mLevelId) + getMD5Hash(HG_ENCRYPT(toEncrypt));
		}

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


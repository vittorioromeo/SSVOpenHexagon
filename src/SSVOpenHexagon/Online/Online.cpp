// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Online/Client.hpp"
#include "SSVOpenHexagon/Online/Server.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/Definitions.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Online/OHServer.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvu::Encryption;
using namespace ssvuj;
using namespace ssvu::FileSystem;

namespace hg
{
	namespace Online
	{
		const IpAddress localIp{"127.0.0.1"};
		const IpAddress hostIp{"46.4.172.228"};
		const unsigned short localPort{54000};
		const unsigned short hostPort{27273};

		ConnectStat connectionStatus{ConnectStat::Disconnected};
		LoginStat loginStatus{LoginStat::Unlogged};
		Online::GlobalThreadManager* currentGtm{nullptr};

		float serverVersion{-1};
		string serverMessage;

		ValidatorDB validators;

		PacketHandler<Client> clientPHandler;
		Uptr<Client> client;

		bool gettingLeaderboard{false}, forceLeaderboardRefresh{false};
		string lastLeaderboardId;
		float lastLeaderboardDM;

		bool newUserReg{false}, needsCleanup{false};

		string currentUsername{"NULL"}, currentLeaderboard{"NULL"}, currentUserStatsStr{"NULL"};
		UserStats currentUserStats;
		ssvuj::Obj currentFriendScores;

		void refreshUserStats()
		{
			ssvuj::Obj root{getFromString(currentUserStatsStr)};
			currentUserStats = ssvuj::getExtr<UserStats>(root);
		}

		void setCurrentGtm(GlobalThreadManager& mGtm) { currentGtm = &mGtm; }

		void initializeClient()
		{
			clientPHandler[FromServer::LoginResponseValid] = [](Client&, Packet& mP)
			{
				lo("PacketHandler") << "Successfully logged in!" << endl;
				loginStatus = LoginStat::Logged;
				newUserReg = ssvuj::getExtr<bool>(getDecompressedPacket(mP), 0);
				trySendInitialRequests();
			};
			clientPHandler[FromServer::LoginResponseInvalid] = [](Client&, Packet&)		{ loginStatus = LoginStat::Unlogged; lo("PacketHandler") << "Login invalid!" << endl; };
			clientPHandler[FromServer::RequestInfoResponse] = [](Client&, Packet& mP)	{ ssvuj::Obj r{getDecompressedPacket(mP)}; serverVersion = ssvuj::getExtr<float>(r, 0); serverMessage = ssvuj::getExtr<string>(r, 1); };
			clientPHandler[FromServer::SendLeaderboard] = [](Client&, Packet& mP)		{ currentLeaderboard = ssvuj::getExtr<string>(getDecompressedPacket(mP), 0); gettingLeaderboard = false; };
			clientPHandler[FromServer::SendLeaderboardFailed] = [](Client&, Packet&)	{ currentLeaderboard = "NULL"; lo("PacketHandler") << "Server failed sending leaderboard" << endl; gettingLeaderboard = false; };
			clientPHandler[FromServer::SendScoreResponseValid] = [](Client&, Packet&)	{ lo("PacketHandler") << "Server successfully accepted score" << endl; };
			clientPHandler[FromServer::SendScoreResponseInvalid] = [](Client&, Packet&)	{ lo("PacketHandler") << "Server refused score" << endl; };
			clientPHandler[FromServer::SendUserStats] = [](Client&, Packet& mP)			{ currentUserStatsStr = ssvuj::getExtr<string>(getDecompressedPacket(mP), 0); refreshUserStats(); };
			clientPHandler[FromServer::SendUserStatsFailed] = [](Client&, Packet&)		{ currentUserStatsStr = "NULL"; lo("PacketHandler") << "Server failed sending user stats" << endl; };
			clientPHandler[FromServer::SendFriendsScores] = [](Client&, Packet& mP)		{ currentFriendScores = ssvuj::getFromString(ssvuj::getExtr<string>(getDecompressedPacket(mP), 0)); };
			clientPHandler[FromServer::SendLogoutValid] = [](Client&, Packet&)			{ loginStatus = LoginStat::Unlogged; };
			clientPHandler[FromServer::NUR_EmailValid] = [](Client&, Packet&)			{ newUserReg = false; };

			client = ssvu::makeUptr<Client>(clientPHandler);

			currentGtm->start([]
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
					if(needsCleanup) return;
					this_thread::sleep_for(chrono::milliseconds(1000));
				}
			});
		}

		bool canSendPacket() { return connectionStatus == ConnectStat::Connected && loginStatus == LoginStat::Logged && currentUsername != "NULL"; }

		template<typename T> void trySendFunc(T mFunc)
		{
			if(!canSendPacket()) { lo("hg::Online::trySendFunc") << "Can't send data to server: not connected / not logged in" << endl; return; }
			if(Config::getServerVerbose()) lo("hg::Online::trySendFunc") << "Sending data to server..." << endl;

			currentGtm->start([mFunc]
			{
				if(!canSendPacket()) { lo("hg::Online::trySendFunc") << "Client not connected - aborting" << endl; return; }
				mFunc();
			});
		}

		template<unsigned int TType, typename... TArgs> void trySendPacket(TArgs&&... mArgs)
		{
			auto packet(buildCPacket<TType>(mArgs...));
			trySendFunc([packet]{ client->send(packet); });
		}

		void tryConnectToServer()
		{
			if(connectionStatus == ConnectStat::Connecting)	{ lo("hg::Online::connectToServer") << "Already connecting" << endl; return; }
			if(connectionStatus == ConnectStat::Connected)	{ lo("hg::Online::connectToServer") << "Already connected" << endl; return; }

			lo("hg::Online::connectToServer") << "Connecting to server..." << endl;
			client->disconnect();
			connectionStatus = ConnectStat::Connecting;

			currentGtm->start([]
			{
				if(client->connect(getCurrentIpAddress(), getCurrentPort()))
				{
					lo("hg::Online::connectToServer") << "Connected to server!" << endl;
					connectionStatus = ConnectStat::Connected; return;
				}

				lo("hg::Online::connectToServer") << "Failed to connect" << endl;
				connectionStatus = ConnectStat::Disconnected;
				client->disconnect();
			});
		}

		void tryLogin(const string& mUsername, const string& mPassword)
		{
			if(loginStatus != LoginStat::Unlogged) { logout(); return; }
			if(connectionStatus != ConnectStat::Connected) { lo("hg::Online::tryLogin") << "Client not connected - aborting" << endl; loginStatus = LoginStat::Unlogged; return; }

			lo("hg::Online::tryLogin") << "Logging in..." << endl;
			loginStatus = LoginStat::Logging;

			currentGtm->start([&mUsername, &mPassword]
			{
				client->send(buildCPacket<FromClient::Login>(mUsername, mPassword));
				currentUsername = mUsername;

				std::this_thread::sleep_for(std::chrono::seconds(6));

				if(loginStatus == LoginStat::Logging)
				{
					lo("hg::Online::tryLogin") << "Failed to login - aborting" << endl; loginStatus = LoginStat::Unlogged; return;
				}
			});
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

		void cleanup() { needsCleanup = true; }

		void requestLeaderboardIfNeeded(const string& mLevelId, float mDiffMult)
		{
			if(!forceLeaderboardRefresh)
			{
				if(gettingLeaderboard || (lastLeaderboardId == mLevelId && lastLeaderboardDM == mDiffMult)) return;
			}
			else forceLeaderboardRefresh = false;

			invalidateCurrentLeaderboard();
			invalidateCurrentFriendsScores();
			gettingLeaderboard = true;
			lastLeaderboardId = mLevelId;
			lastLeaderboardDM = mDiffMult;
			tryRequestLeaderboard(mLevelId, mDiffMult);
			tryRequestFriendsScores(mLevelId, mDiffMult);
			trySendPacket<FromClient::RequestUserStats>(currentUsername);
		}
		void setForceLeaderboardRefresh(bool mValue) { forceLeaderboardRefresh = mValue; }

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

		string getValidator(const Path& mPackPath, const string& mLevelId, const string& mLevelRootString, const Path& mStyleRootPath, const Path& mLuaScriptPath)
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
				Path path{mPackPath + "/Scripts/" + lsn};
				toEncrypt += getFileContents(path);
			}

			toEncrypt = getControlStripped(toEncrypt);
			return getUrlEncoded(mLevelId) + getMD5Hash(HG_ENCRYPT(toEncrypt));
		}

		string getMD5Hash(const string& mStr)	{ return encrypt<Encryption::Type::MD5>(mStr); }

		void initializeValidators(HGAssets& mAssets)
		{
			if(Config::getServerVerbose()) lo("hg::Online::initializeValidators") << "Initializing validators..." << endl;

			for(const auto& p : mAssets.getLevelDatas())
			{
				if(Config::getServerVerbose()) lo("hg::Online::initializeValidators") << "Adding (" << p.first << ") validator" << endl;

				const auto& l(p.second);
				const auto& validator(getValidator(l->packPath, l->id, l->getRootString(), mAssets.getStyleData(l->styleId).getRootPath(), l->luaScriptPath));
				validators.addValidator(p.first, validator);

				if(Config::getServerVerbose()) lo("hg::Online::initializeValidators") << "Added (" << p.first << "): " << validator << endl;
			}

			lo("hg::Online::initializeValidators") << "Finished initializing validators..." << endl;
		}

		const sf::IpAddress& getCurrentIpAddress()	{ return Config::getServerLocal() ? localIp : hostIp; }
		unsigned short getCurrentPort()				{ return Config::getServerLocal() ? localPort : hostPort; }
	}
}


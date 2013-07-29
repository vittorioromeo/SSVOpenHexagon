// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <functional>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Server.h"
#include "SSVOpenHexagon/Online/Client.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Online/Definitions.h"
#include "SSVOpenHexagon/Utils/Utils.h"

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
		using Request = Http::Request;
		using Response = Http::Response;
		using Status = Http::Response::Status;

		const IpAddress hostIp{"209.236.124.147"};
		const unsigned short hostPort{27272};

		bool connected{false};
		bool loggedIn{false};

		float serverVersion{-1};
		string serverMessage{""};


		PacketHandler clientPHandler;
		Uptr<Client> client;

		string currentLeaderboard{"NULL"};

		template<unsigned int TType> Packet buildPacket() { Packet result; result << TType; return result; }

		template<typename TArg> void buildHelper(Packet& mP, TArg&& mArg) { mP << mArg; }
		template<typename TArg, typename... TArgs> void buildHelper(Packet& mP, TArg&& mArg, TArgs&&... mArgs) { mP << mArg; buildHelper(mP, mArgs...); }
		template<unsigned int TType, typename... TArgs> Packet buildPacket(TArgs&&... mArgs) { Packet result; result << TType; buildHelper(result, mArgs...); return result; }




		void initializeServer()
		{
			const std::string secretKey{"temp"};
			UserDB users;
			ScoreDB scores;

			const auto& usersPath("users.json");
			users = ssvuj::as<UserDB>(ssvuj::getRootFromFile(usersPath));
			auto saveUsers = [&]{ ssvuj::Value root; ssvuj::set(root, users); ssvuj::writeRootToFile(root, usersPath); };


			const auto& scoresPath("scores.json");
			scores = ssvuj::as<ScoreDB>(ssvuj::getRootFromFile(scoresPath));
			auto saveScores = [&]{ ssvuj::Value root; ssvuj::set(root, scores); ssvuj::writeRootToFile(root, scoresPath); };

			saveScores(); // PROBLEM HERE


			PacketHandler pHandler;
			pHandler[ClientPackets::Ping] = [](ManagedSocket&, sf::Packet&) { };
			pHandler[ClientPackets::Login] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				// Validate login information, then send a response

				string username, password;
				mP >> username >> password;

				string passwordHash{Encryption::encrypt<Encryption::Type::MD5>(password + secretKey)};

				if(users.hasUser(username))
				{
					const auto& u(users.getUser(username));
					lo << lt("PacketHandler") << "Username found" << endl;

					if(u.isPasswordHash(passwordHash))
					{
						lo << lt("PacketHandler") << "Password valid" << endl;
						mMS.send(buildPacket<ServerPackets::LoginResponseValid>());
					}
					else
					{
						lo << lt("PacketHandler") << "Password invalid" << endl;
						mMS.send(buildPacket<ServerPackets::LoginResponseInvalid>());
					}
				}
				else
				{
					lo << lt("PacketHandler") << "Username not found, registering" << endl;

					User newUser; newUser.setPasswordHash(passwordHash);
					users.registerUser(username, newUser);

					saveUsers();

					lo << lt("PacketHandler") << "Accepting new user" << endl;
					mMS.send(buildPacket<ServerPackets::LoginResponseValid>());
				}
			};
			pHandler[ClientPackets::RequestInfo] = [](ManagedSocket& mMS, sf::Packet&)
			{
				// Return info from server

				float version{2.f};
				string message{"Welcome to Open Hexagon 2.0!"};

				mMS.send(buildPacket<ServerPackets::RequestInfoResponse>(version, message));
			};
			pHandler[ClientPackets::SendScore] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				string username, levelId, validator; float diffMult, score;
				mP >> username >> levelId >> validator >> diffMult >> score;

				if(!scores.hasLevel(levelId))
				{
					lo << lt("PacketHandler") << "No table for this level id, creating one" << endl;
					LevelScoreDB db; db.setValidator("0");
					scores.addLevel(levelId, db);
				}

				auto& l(scores.getLevel(levelId));

				if(!l.isValidator(validator))
				{
					lo << lt("PacketHandler") << "Validator doesn't match" << endl;
					mMS.send(buildPacket<ServerPackets::SendScoreResponseInvalid>());
					return;
				}

				lo << lt("PacketHandler") << "Validator matches, inserting score" << endl;
				if(l.getScore(diffMult, username) < score) l.addScore(diffMult, username, score);

				saveScores();
				mMS.send(buildPacket<ServerPackets::SendScoreResponseValid>());
			};
			pHandler[ClientPackets::RequestLeaderboard] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				string username, levelId, validator; float diffMult;
				mP >> username >> levelId >> validator >> diffMult;

				if(!scores.hasLevel(levelId)) { mMS.send(buildPacket<ServerPackets::SendLeaderboardFailed>()); return; }
				auto& l(scores.getLevel(levelId));

				if(!l.isValidator(validator) || !l.hasDiffMult(diffMult)) { mMS.send(buildPacket<ServerPackets::SendLeaderboardFailed>()); return; }
				lo << lt("PacketHandler") << "Validator matches, sending leaderboard" << endl;

				const auto& sortedScores(l.getSortedScores(diffMult));
				ssvuj::Value response;

				unsigned int i{0};
				for(const auto& v : sortedScores)
				{
					ssvuj::set(response["r"][i], 0, v.second);
					ssvuj::set(response["r"][i], 1, v.first);
					++i;
					if(i > getClamped(8u, 0u, static_cast<unsigned int>(sortedScores.size()))) break;
				}
				ssvuj::set(response, "id", levelId);

				float playerScore{l.getPlayerScore(username, diffMult)};
				playerScore == -1 ? ssvuj::set(response, "ps", "NULL") : ssvuj::set(response, "ps", playerScore);

				string leaderboardDataString;
				ssvuj::writeRootToString(response, leaderboardDataString);
				mMS.send(buildPacket<ServerPackets::SendLeaderboard>(leaderboardDataString));
			};


			Server server{pHandler};
			server.onClientAccepted += [&](ClientHandler&){ };

			server.start(54000);
			while(true) this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		void initializeClient()
		{
			clientPHandler[ServerPackets::LoginResponseValid] = [](ManagedSocket& mMS, sf::Packet&)		{ lo << lt("PacketHandler") << "Successfully logged in!" << endl; loggedIn = true; mMS.send(buildPacket<ClientPackets::RequestInfo>()); };
			clientPHandler[ServerPackets::LoginResponseInvalid] = [](ManagedSocket&, sf::Packet&)		{ loggedIn = false; lo << lt("PacketHandler") << "Login invalid!" << endl; };
			clientPHandler[ServerPackets::RequestInfoResponse] = [](ManagedSocket&, sf::Packet& mP)		{ mP >> serverVersion >> serverMessage; };
			clientPHandler[ServerPackets::SendLeaderboard] = [](ManagedSocket&, sf::Packet& mP)			{ mP >> currentLeaderboard; };
			clientPHandler[ServerPackets::SendLeaderboardFailed] = [](ManagedSocket&, sf::Packet&)		{ currentLeaderboard = "NULL"; };

			client = Uptr<Client>(new Client(clientPHandler));

			thread([]
			{
				while(true)
				{
					if(connected) { client->send(buildPacket<ClientPackets::Ping>()); }
					this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}).detach();
		}

		void tryConnectToServer()
		{
			lo << lt("hg::Online::connectToServer") << "Connecting to server..." << endl;

			thread([]
			{
				if(client->connect("127.0.0.1", 54000))
				{
					lo << lt("hg::Online::connectToServer") << "Connected to server!" << endl;
					connected = true; return;
				}

				lo << lt("hg::Online::connectToServer") << "Failed to connect" << endl;
				connected = false;
			}).detach();
		}
		bool isConnected() { return connected; }

		void tryLogin(const string& mUsername, const string& mPassword)
		{
			lo << lt("hg::Online::tryLogin") << "Logging in..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(100));
				if(!connected) { lo << lt("hg::Online::tryLogin") << "Client not connected - aborting" << endl; return; }
				client->send(buildPacket<ClientPackets::Login>(mUsername, mPassword));
			}).detach();
		}
		bool isLoggedIn() { return loggedIn; }

		void trySendScore(const string& mUsername, const string& mLevelId, const string& mValidator, float mDiffMult, float mScore)
		{
			lo << lt("hg::Online::trySendScore") << "Sending score..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(100));
				if(!connected) { lo << lt("hg::Online::trySendScore") << "Client not connected - aborting" << endl; return; }
				client->send(buildPacket<ClientPackets::SendScore>(mUsername, mLevelId, mValidator, mDiffMult, mScore));
			}).detach();
		}

		void tryRequestLeaderboard(const std::string& mUsername, const string& mLevelId, const string& mValidator, float mDiffMult)
		{
			lo << lt("hg::Online::tryRequestLeaderboard") << "Requesting scores..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(100));
				if(!connected) { lo << lt("hg::Online::tryRequestLeaderboard") << "Client not connected - aborting" << endl; return; }
				client->send(buildPacket<ClientPackets::RequestLeaderboard>(mUsername, mLevelId, mValidator, mDiffMult));
			}).detach();
		}

		void invalidateCurrentLeaderboard() { currentLeaderboard = "NULL"; }
		const string& getCurrentLeaderboard() { return currentLeaderboard; }


		string getValidator(const string& mPackPath, const string& mLevelId, const string& mLevelRootPath, const string& mStyleRootPath, const string& mLuaScriptPath)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt{""};
			toEncrypt.append(mLevelId);
			toEncrypt.append(getFileContents(mLevelRootPath));
			toEncrypt.append(getFileContents(mStyleRootPath));
			toEncrypt.append(luaScriptContents);

			for(const auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName};
				string contents{getFileContents(path)};
				toEncrypt.append(contents);
			}

			toEncrypt = getControlStripped(toEncrypt);

			string result{getUrlEncoded(mLevelId) + getMD5Hash(toEncrypt + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};
			return result;
		}

		float getServerVersion() 							{ return serverVersion; }
		string getServerMessage() 							{ return serverMessage; }
		string getMD5Hash(const string& mString) 			{ return encrypt<Encryption::Type::MD5>(mString); }


	}
}


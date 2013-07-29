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

			const auto& usersPath("users.json");
			ssvuj::Value users{ssvuj::getRootFromFile(usersPath)};
			auto saveUsers = [&]{ ssvuj::writeRootToFile(users, usersPath); };

			const auto& scoresPath("scores.json");
			ssvuj::Value scores{ssvuj::getRootFromFile(scoresPath)};
			auto saveScores = [&]{ ssvuj::writeRootToFile(scores, scoresPath); };

			PacketHandler pHandler;
			pHandler[ClientPackets::Ping] = [](ManagedSocket&, sf::Packet&) { };
			pHandler[ClientPackets::Login] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				// Validate login information, then send a response

				string username, password;
				mP >> username >> password;

				string passwordHash{Encryption::encrypt<Encryption::Type::MD5>(password + secretKey)};

				if(ssvuj::has(users, username))
				{
					lo << lt("PacketHandler") << "Username found" << endl;

					const auto& userData(ssvuj::as<ssvuj::Value>(users, username));
					if(as<string>(userData, "passwordHash") == passwordHash)
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
					users[username] = ssvuj::Value{};
					ssvuj::set(users[username], "passwordHash", passwordHash);

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

				if(!ssvuj::has(scores, levelId))
				{
					lo << lt("PacketHandler") << "No table for this level id, creating one" << endl;
					scores[levelId] = ssvuj::Value{};
					ssvuj::set(scores[levelId]["validator"], /* calc validator here */ "0");
				}

				if(ssvuj::as<string>(scores[levelId], "validator") == validator)
				{
					lo << lt("PacketHandler") << "Validator matches, inserting score" << endl;

					if(!ssvuj::has(scores[levelId], toStr(diffMult))) scores[levelId][toStr(diffMult)] = ssvuj::Value{};

					ssvuj::Value scoreData;
					ssvuj::set(scoreData, 0, username);
					ssvuj::set(scoreData, 1, score);

					auto& d(scores[levelId][toStr(diffMult)]);
					if(ssvuj::size(d) == 0) ssvuj::append(d, scoreData);
					else
					{
						for(unsigned int i{0}; i < ssvuj::size(d); ++i)
						{
							if(d[i][0] == username)
							{
								if(d[i][1] < score)	{ ssvuj::erase(d, i); break; }
								else { mMS.send(buildPacket<ServerPackets::SendScoreResponseValid>()); return; }
							}
						}

						bool found{false};
						for(unsigned int i{0}; i < ssvuj::size(d); ++i)
						{
							if(score > as<float>(d[i], 1)) { ssvuj::insert(d, i, scoreData); found = true; break; }
						}

						if(!found) ssvuj::append(scores[levelId][toStr(diffMult)], scoreData);
					}

					saveScores();

					mMS.send(buildPacket<ServerPackets::SendScoreResponseValid>());
				}
				else
				{
					lo << lt("PacketHandler") << "Validator doesn't match" << endl;
					mMS.send(buildPacket<ServerPackets::SendScoreResponseInvalid>());
				}
			};
			pHandler[ClientPackets::RequestLeaderboard] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				string username, levelId, validator; float diffMult;
				mP >> username >> levelId >> validator >> diffMult;

				if(!ssvuj::has(scores, levelId)) { mMS.send(buildPacket<ServerPackets::SendLeaderboardFailed>()); return; }

				if(ssvuj::as<string>(scores[levelId], "validator") == validator)
				{
					lo << lt("PacketHandler") << "Validator matches, sending leaderboard" << endl;

					if(!ssvuj::has(scores[levelId], toStr(diffMult))) scores[levelId][toStr(diffMult)] = ssvuj::Value{};
					ssvuj::Value leaderboardData;
					const auto& d(scores[levelId][toStr(diffMult)]);

					for(unsigned int i{0}; i < ssvu::getClamped(5u, 0u, ssvuj::size(d)); ++i) leaderboardData["records"].append(ssvuj::as<ssvuj::Value>(d[i]));
					ssvuj::set(leaderboardData, "levelId", levelId);
					ssvuj::set(leaderboardData, "playerScore", "NULL");

					for(unsigned int i{0}; i < ssvuj::size(d); ++i) if(as<string>(d[i], 0) == username) { ssvuj::set(leaderboardData, "playerScore", as<float>(d[i], 1)); break; }

					string leaderboardDataString;
					ssvuj::writeRootToString(leaderboardData, leaderboardDataString);
					mMS.send(buildPacket<ServerPackets::SendLeaderboard>(leaderboardDataString));
				}
				else mMS.send(buildPacket<ServerPackets::SendLeaderboardFailed>());
			};


			Server server{pHandler};
			std::vector<Uptr<ClientData>> clientDatas;
			server.onClientAccepted += [&](ClientHandler&){ };

			server.start(54000);
			while(true) this_thread::sleep_for(std::chrono::milliseconds(100));
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
				this_thread::sleep_for(std::chrono::milliseconds(1000));
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
				this_thread::sleep_for(std::chrono::milliseconds(1000));
				if(!connected) { lo << lt("hg::Online::trySendScore") << "Client not connected - aborting" << endl; return; }
				client->send(buildPacket<ClientPackets::SendScore>(mUsername, mLevelId, mValidator, mDiffMult, mScore));
			}).detach();
		}

		void tryRequestLeaderboard(const std::string& mUsername, const string& mLevelId, const string& mValidator, float mDiffMult)
		{
			lo << lt("hg::Online::tryRequestLeaderboard") << "Requesting scores..." << endl;

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(1000));
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


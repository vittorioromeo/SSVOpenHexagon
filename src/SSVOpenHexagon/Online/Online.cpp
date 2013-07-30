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
#include "SSVOpenHexagon/Online/Compression.h"
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
		using Request = Http::Request;
		using Response = Http::Response;
		using Status = Http::Response::Status;

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

		namespace Internal
		{
			template<typename TArg> void pBuildHelper(Packet& mP, TArg&& mArg) { mP << mArg; }
			template<typename TArg, typename... TArgs> void pBuildHelper(Packet& mP, TArg&& mArg, TArgs&&... mArgs) { mP << mArg; pBuildHelper(mP, mArgs...); }

			template<unsigned int TIndex, typename TArg> void jBuildHelper(ssvuj::Value& mP, TArg&& mArg) { ssvuj::set(mP, TIndex, mArg); }
			template<unsigned int TIndex, typename TArg, typename... TArgs> void jBuildHelper(ssvuj::Value& mP, TArg&& mArg, TArgs&&... mArgs) { ssvuj::set(mP, TIndex, mArg); jBuildHelper<TIndex + 1>(mP, mArgs...); }

			ssvuj::Value getDecompressedJsonString(const string& mData) { ssvuj::Value result{getRootFromString(getZLIBDecompress(mData))}; return result; }
			template<typename... TArgs> ssvuj::Value buildJsonDataArray(TArgs&&... mArgs) { ssvuj::Value result; Internal::jBuildHelper<0>(result, mArgs...); return result; }
			template<typename... TArgs> string buildCompressedJsonString(TArgs&&... mArgs)
			{
				ssvuj::Value request{buildJsonDataArray(mArgs...)}; string requestString;
				ssvuj::writeRootToString(request, requestString); return getZLIBCompress(requestString);
			}
		}
		template<unsigned int TType> Packet buildPacket() { Packet result; result << TType; return result; }
		template<unsigned int TType, typename... TArgs> Packet buildPacket(TArgs&&... mArgs) { Packet result; result << TType; Internal::pBuildHelper(result, mArgs...); return result; }

		template<unsigned int TType, typename... TArgs> Packet buildCompressedPacket(TArgs&&... mArgs) { Packet result; result << TType << Internal::buildCompressedJsonString(mArgs...); return result; }
		ssvuj::Value getDecompressedPacket(Packet& mPacket) { string data; mPacket >> data; return Internal::getDecompressedJsonString(data); }


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

			PacketHandler pHandler;
			pHandler[FromClient::Ping] = [](ManagedSocket&, sf::Packet&) { };
			pHandler[FromClient::Login] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				ssvuj::Value request{getDecompressedPacket(mP)};

				string username{ssvuj::as<string>(request, 0)}, password{ssvuj::as<string>(request, 1)};
				string passwordHash{getMD5Hash(password + secretKey)};

				if(users.hasUser(username))
				{
					const auto& u(users.getUser(username));
					lo << lt("PacketHandler") << "Username found" << endl;

					if(u.isPasswordHash(passwordHash))
					{
						lo << lt("PacketHandler") << "Password valid" << endl;
						mMS.send(buildPacket<FromServer::LoginResponseValid>());
						return;
					}

					lo << lt("PacketHandler") << "Password invalid" << endl;
					mMS.send(buildPacket<FromServer::LoginResponseInvalid>());
				}
				else
				{
					lo << lt("PacketHandler") << "Username not found, registering" << endl;

					User newUser; newUser.setPasswordHash(passwordHash);
					users.registerUser(username, newUser);

					saveUsers();

					lo << lt("PacketHandler") << "Accepting new user" << endl;
					mMS.send(buildPacket<FromServer::LoginResponseValid>());
				}
			};
			pHandler[FromClient::RequestInfo] = [](ManagedSocket& mMS, sf::Packet&)
			{
				float version{2.f};
				string message{"Welcome to Open Hexagon 2.0!"};

				mMS.send(buildCompressedPacket<FromServer::RequestInfoResponse>(version, message));
			};
			pHandler[FromClient::SendScore] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				ssvuj::Value request{getDecompressedPacket(mP)};

				string username{ssvuj::as<string>(request, 0)}, levelId{ssvuj::as<string>(request, 1)}, validator{ssvuj::as<string>(request, 2)};
				float diffMult{ssvuj::as<float>(request, 3)}, score{ssvuj::as<float>(request, 4)};

				if(!scores.hasLevel(levelId))
				{
					lo << lt("PacketHandler") << "No table for this level id, creating one" << endl;
					scores.addLevel(levelId, {});
				}

				if(validators.getValidator(levelId) != validator)
				{
					lo << lt("PacketHandler") << "Validator doesn't match" << endl;
					mMS.send(buildPacket<FromServer::SendScoreResponseInvalid>());
					return;
				}

				lo << lt("PacketHandler") << "Validator matches, inserting score" << endl;

				auto& l(scores.getLevel(levelId));
				if(l.getScore(diffMult, username) < score) l.addScore(diffMult, username, score);

				saveScores();
				mMS.send(buildPacket<FromServer::SendScoreResponseValid>());
			};
			pHandler[FromClient::RequestLeaderboard] = [&](ManagedSocket& mMS, sf::Packet& mP)
			{
				ssvuj::Value request{getDecompressedPacket(mP)};

				string username{ssvuj::as<string>(request, 0)}, levelId{ssvuj::as<string>(request, 1)}, validator{ssvuj::as<string>(request, 2)};
				float diffMult{ssvuj::as<float>(request, 3)};

				if(!scores.hasLevel(levelId)) { mMS.send(buildPacket<FromServer::SendLeaderboardFailed>()); return; }
				auto& l(scores.getLevel(levelId));

				if(validators.getValidator(levelId) != validator || !l.hasDiffMult(diffMult)) { mMS.send(buildPacket<FromServer::SendLeaderboardFailed>()); return; }
				lo << lt("PacketHandler") << "Validator matches, sending leaderboard" << endl;

				const auto& sortedScores(l.getSortedScores(diffMult));
				ssvuj::Value response;

				unsigned int i{0};
				for(const auto& v : sortedScores)
				{
					ssvuj::set(response["r"][i], 0, v.second); ssvuj::set(response["r"][i], 1, v.first);
					++i;
					if(i > getClamped(8u, 0u, static_cast<unsigned int>(sortedScores.size()))) break;
				}
				ssvuj::set(response, "id", levelId);

				float playerScore{l.getPlayerScore(username, diffMult)};
				playerScore == -1 ? ssvuj::set(response, "ps", "NULL") : ssvuj::set(response, "ps", playerScore);

				string leaderboardDataString;
				ssvuj::writeRootToString(response, leaderboardDataString);
				mMS.send(buildCompressedPacket<FromServer::SendLeaderboard>(leaderboardDataString));
			};


			Server server{pHandler};
			server.onClientAccepted += [&](ClientHandler&){ };

			server.start(54000);
			while(true) this_thread::sleep_for(std::chrono::milliseconds(10));
		}

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


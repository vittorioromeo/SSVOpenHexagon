#ifndef HG_ONLINE_OHSERVER
#define HG_ONLINE_OHSERVER

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>
#include <SFML/Network.hpp>
#include "SSVOpenHexagon/Online/PacketHandler.h"
#include "SSVOpenHexagon/Online/Server.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Definitions.h"

namespace hg
{
	namespace Online
	{
		struct UserStats
		{
			unsigned int minutesSpentPlaying{0}; // msp
			unsigned int deaths{0}; // dth
			unsigned int restarts{0}; // rst
			std::vector<std::string> trackedNames; // tn
		};
		struct User
		{
			std::string passwordHash; // ph
			UserStats stats; // st
		};
		class UserDB
		{
			private:
				std::unordered_map<std::string, User> users;

			public:
				inline bool hasUser(const std::string& mUsername) const { return users.count(mUsername) > 0; }
				inline User& getUser(const std::string& mUsername) { return users[mUsername]; }
				inline void registerUser(const std::string& mUsername, const User& mUser) { users[mUsername] = mUser; }
				inline const std::unordered_map<std::string, User>& getUsers() const { return users; }
		};
		class LevelScoreDB
		{
			private:
				std::unordered_map<float, std::unordered_map<std::string, float>> scores;
				std::unordered_map<float, std::map<float, std::string>> sortedScores;
				std::unordered_map<float, std::unordered_map<std::string, int>> userPositions;

			public:
				inline void addScore(float mDiffMult, const std::string& mUsername, float mScore)
				{
					scores[mDiffMult][mUsername] = mScore;

					auto& ss(sortedScores[mDiffMult]);
					for(auto itr(std::begin(ss)); itr != std::end(ss); ++itr) if(itr->second == mUsername) { ss.erase(itr); break; }
					ss.insert({mScore, mUsername});

					unsigned int p{1};
					for(auto itr(std::begin(ss)); itr != std::end(ss); ++itr) { userPositions[mDiffMult][mUsername] = p; ++p; }
				}
				inline bool hasDiffMult(float mDiffMult) const { return scores.count(mDiffMult) > 0; }
				inline float getScore(float mDiffMult, const std::string& mUsername) const { if(!hasDiffMult(mDiffMult) || scores.at(mDiffMult).count(mUsername) == 0) return -1; return scores.at(mDiffMult).at(mUsername); }
				inline const std::unordered_map<float, std::unordered_map<std::string, float>>& getScores() const { return scores; }
				inline const std::unordered_map<std::string, float>& getScores(float mDiffMult) const { return scores.at(mDiffMult); }
				inline float getPlayerScore(const std::string& mUsername, float mDiffMult) const { if(!hasDiffMult(mDiffMult) || scores.at(mDiffMult).count(mUsername) == 0) return -1.f; return scores.at(mDiffMult).at(mUsername); }
				inline const std::map<float, std::string>& getSortedScores(float mDiffMult) const { return sortedScores.at(mDiffMult); }
				inline int getPlayerPosition(const std::string& mUsername, float mDiffMult) const { if(!hasDiffMult(mDiffMult) || userPositions.at(mDiffMult).count(mUsername) == 0) return -1; return userPositions.at(mDiffMult).at(mUsername); }

		};
		class ScoreDB
		{
			private:
				std::unordered_map<std::string, LevelScoreDB> levels;

			public:
				inline bool hasLevel(const std::string& mId) const { return levels.count(mId) > 0; }
				inline LevelScoreDB& getLevel(const std::string& mId) { return levels[mId]; }
				inline void addLevel(const std::string& mId, const LevelScoreDB& mDB) { levels[mId] = mDB; }
				inline const std::unordered_map<std::string, LevelScoreDB>& getLevels() const { return levels; }
		};
	}
}

namespace ssvuj
{
	namespace Internal
	{
		template<> struct AsHelper<hg::Online::UserStats>
		{
			inline static hg::Online::UserStats as(const Impl& mValue)
			{
				hg::Online::UserStats result;
				result.deaths = ssvuj::as<long>(mValue, "dth");
				result.minutesSpentPlaying = ssvuj::as<long>(mValue, "msp");
				result.restarts = ssvuj::as<long>(mValue, "rst");
				result.trackedNames = ssvuj::as<std::vector<std::string>>(mValue, "tn");
				return result;
			}
		};

		template<> struct AsHelper<hg::Online::User>
		{
			inline static hg::Online::User as(const Impl& mValue)
			{
				hg::Online::User result;
				result.passwordHash = ssvuj::as<std::string>(mValue, "ph");
				result.stats = ssvuj::as<hg::Online::UserStats>(mValue, "st");
				return result;
			}
		};

		template<> struct AsHelper<hg::Online::UserDB>
		{
			inline static hg::Online::UserDB as(const Impl& mValue)
			{
				hg::Online::UserDB result;
				for(auto itr(std::begin(mValue)); itr != std::end(mValue); ++itr) result.registerUser(ssvuj::as<std::string>(itr.key()), ssvuj::as<hg::Online::User>(*itr));
				return result;
			}
		};

		template<> struct AsHelper<hg::Online::LevelScoreDB>
		{
			inline static hg::Online::LevelScoreDB as(const Impl& mValue)
			{
				hg::Online::LevelScoreDB result;

				for(auto itr(std::begin(mValue)); itr != std::end(mValue); ++itr)
				{
					if(ssvuj::as<std::string>(itr.key()) == "validator") continue;
					for(unsigned int i{0}; i < ssvuj::size(*itr); ++i) result.addScore(std::stof(ssvuj::as<std::string>(itr.key())), ssvuj::as<std::string>((*itr)[i], 0), ssvuj::as<float>((*itr)[i], 1));
				}

				return result;
			}
		};

		template<> struct AsHelper<hg::Online::ScoreDB>
		{
			inline static hg::Online::ScoreDB as(const Impl& mValue)
			{
				hg::Online::ScoreDB result;

				for(auto itr(std::begin(mValue)); itr != std::end(mValue); ++itr)
					result.addLevel(ssvuj::as<std::string>(itr.key()), ssvuj::as<hg::Online::LevelScoreDB>(*itr));

				return result;
			}
		};
	}

	template<> inline void set<hg::Online::UserStats>(Impl& mRoot, const hg::Online::UserStats& mValueToSet)
	{
		set(mRoot, "dth", mValueToSet.deaths);
		set(mRoot, "msp", mValueToSet.minutesSpentPlaying);
		set(mRoot, "rst", mValueToSet.restarts);

		for(unsigned int i{0}; i < mValueToSet.trackedNames.size(); ++i) set(mRoot["tn"], i, mValueToSet.trackedNames[i]);
	}

	template<> inline void set<hg::Online::User>(Impl& mRoot, const hg::Online::User& mValueToSet)
	{
		set(mRoot, "ph", mValueToSet.passwordHash);
		set(mRoot, "st", mValueToSet.stats);
	}

	template<> inline void set<hg::Online::UserDB>(Impl& mRoot, const hg::Online::UserDB& mValueToSet)
	{
		for(const auto p : mValueToSet.getUsers()) ssvuj::set(mRoot, p.first, p.second);
	}

	template<> inline void set<hg::Online::LevelScoreDB>(Impl& mRoot, const hg::Online::LevelScoreDB& mValueToSet)
	{
		for(const auto& s : mValueToSet.getScores())
		{
			unsigned int i{0};
			for(const auto& r : s.second)
			{
				ssvuj::Value temp; ssvuj::set(temp, 0, r.first); ssvuj::set(temp, 1, r.second);
				ssvuj::set(mRoot[ssvu::toStr(s.first)], i, temp);
				++i;
			}
		}
	}

	template<> inline void set<hg::Online::ScoreDB>(Impl& mRoot, const hg::Online::ScoreDB& mValueToSet)
	{
		for(const auto& l : mValueToSet.getLevels()) ssvuj::set(mRoot, l.first, l.second);
	}
}

namespace hg
{
	namespace Online
	{
		struct OHServer
		{
			const std::string usersPath{"users.json"};
			const std::string scoresPath{"scores.json"};

			UserDB users{ssvuj::as<UserDB>(ssvuj::getRootFromFile(usersPath))};
			ScoreDB scores{ssvuj::as<ScoreDB>(ssvuj::getRootFromFile(scoresPath))};
			PacketHandler pHandler;
			Server server{pHandler};

			std::unordered_map<unsigned int, std::string> loggedUids;
			std::unordered_set<std::string> loggedUsers;

			inline bool isLoggedIn(const std::string& mUsername) { return loggedUsers.find(mUsername) != std::end(loggedUsers); }
			inline void acceptLogin(unsigned int mUid, const std::string& mUsername) { loggedUids[mUid] = mUsername; loggedUsers.insert(mUsername); }

			inline void saveUsers()	const	{ ssvuj::Value root; ssvuj::set(root, users); ssvuj::writeRootToFile(root, usersPath); }
			inline void saveScores() const	{ ssvuj::Value root; ssvuj::set(root, scores); ssvuj::writeRootToFile(root, scoresPath); }

			inline void forceLogout(unsigned int mUid)
			{
				const auto& itr(loggedUids.find(mUid));
				if(itr == std::end(loggedUids)) return;
				auto username(itr->second);
				loggedUsers.erase(username);
				loggedUids.erase(itr);
				ssvu::lo << ssvu::lt("Forced logout") << username << std::endl;
			}
			inline void logout(const std::string& mUsername)
			{
				loggedUsers.erase(mUsername);
				for(auto itr(std::begin(loggedUids)); itr != std::end(loggedUids); ++itr) if(itr->second == mUsername) { loggedUids.erase(itr); break; }
				ssvu::lo << ssvu::lt("Logout") << mUsername << std::endl;
			}

			OHServer()
			{
				server.onClientAccepted += [&](ClientHandler& mCH)
				{
					mCH.send(buildCompressedPacket<FromServer::ClientAccepted>(mCH.getUid()));
					mCH.onDisconnect += [&]{ forceLogout(mCH.getUid()); };
				};
				pHandler[FromClient::Ping] = [](ManagedSocket&, sf::Packet&) { };
				pHandler[FromClient::Login] = [&](ManagedSocket& mMS, sf::Packet& mP)
				{
					ssvuj::Value request{getDecompressedPacket(mP)};

					unsigned int clientUid{ssvuj::as<unsigned int>(request, 0)};
					std::string username{ssvuj::as<std::string>(request, 1)};

					if(isLoggedIn(username))
					{
						ssvu::lo << ssvu::lt("PacketHandler") << "User already logged in" << std::endl;
						mMS.send(buildPacket<FromServer::LoginResponseInvalid>());
						return;
					}

					std::string password{ssvuj::as<std::string>(request, 2)};
					std::string passwordHash{getMD5Hash(password + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};

					if(users.hasUser(username))
					{
						const auto& u(users.getUser(username));
						ssvu::lo << ssvu::lt("PacketHandler") << "Username found" << std::endl;

						if(u.passwordHash == passwordHash)
						{
							ssvu::lo << ssvu::lt("PacketHandler") << "Password valid" << std::endl;
							acceptLogin(clientUid, username);
							mMS.send(buildPacket<FromServer::LoginResponseValid>());
							return;
						}

						ssvu::lo << ssvu::lt("PacketHandler") << "Password invalid" << std::endl;
						mMS.send(buildPacket<FromServer::LoginResponseInvalid>());
						return;
					}

					ssvu::lo << ssvu::lt("PacketHandler") << "Username not found, registering" << std::endl;

					User newUser; newUser.passwordHash = passwordHash;
					users.registerUser(username, newUser);

					saveUsers();

					ssvu::lo << ssvu::lt("PacketHandler") << "Accepting new user" << std::endl;
					acceptLogin(clientUid, username);
					mMS.send(buildPacket<FromServer::LoginResponseValid>());
				};
				pHandler[FromClient::RequestInfo] = [](ManagedSocket& mMS, sf::Packet&)
				{
					float version{2.f};
					std::string message{"Welcome to Open Hexagon 2.0!"};

					mMS.send(buildCompressedPacket<FromServer::RequestInfoResponse>(version, message));
				};
				pHandler[FromClient::SendScore] = [&](ManagedSocket& mMS, sf::Packet& mP)
				{
					ssvuj::Value request{getDecompressedPacket(mP)};

					std::string username{ssvuj::as<std::string>(request, 0)};
					if(!isLoggedIn(username)) { mMS.send(buildPacket<FromServer::SendScoreResponseInvalid>()); return; }

					std::string levelId{ssvuj::as<std::string>(request, 1)}, validator{ssvuj::as<std::string>(request, 2)};
					float diffMult{ssvuj::as<float>(request, 3)}, score{ssvuj::as<float>(request, 4)};

					if(!scores.hasLevel(levelId))
					{
						ssvu::lo << ssvu::lt("PacketHandler") << "No table for this level id, creating one" << std::endl;
						scores.addLevel(levelId, {});
					}

					if(Online::getValidators().getValidator(levelId) != validator)
					{
						ssvu::lo << ssvu::lt("PacketHandler") << "Validator doesn't match" << std::endl;
						mMS.send(buildPacket<FromServer::SendScoreResponseInvalid>());
						return;
					}

					// ssvu::lo << ssvu::lt("PacketHandler") << "Validator matches, inserting score" << std::endl;

					auto& l(scores.getLevel(levelId));
					if(l.getScore(diffMult, username) < score) l.addScore(diffMult, username, score);

					saveScores();
					mMS.send(buildPacket<FromServer::SendScoreResponseValid>());
				};
				pHandler[FromClient::RequestLeaderboard] = [&](ManagedSocket& mMS, sf::Packet& mP)
				{
					ssvuj::Value request{getDecompressedPacket(mP)};

					std::string username{ssvuj::as<std::string>(request, 0)};
					if(!isLoggedIn(username)) { mMS.send(buildPacket<FromServer::SendLeaderboardFailed>()); return; }

					std::string levelId{ssvuj::as<std::string>(request, 1)}, validator{ssvuj::as<std::string>(request, 2)};
					float diffMult{ssvuj::as<float>(request, 3)};

					if(!scores.hasLevel(levelId)) { mMS.send(buildPacket<FromServer::SendLeaderboardFailed>()); return; }
					auto& l(scores.getLevel(levelId));

					if(Online::getValidators().getValidator(levelId) != validator || !l.hasDiffMult(diffMult)) { mMS.send(buildPacket<FromServer::SendLeaderboardFailed>()); return; }
					// ssvu::lo << ssvu::lt("PacketHandler") << "Validator matches, sending leaderboard" << std::endl;

					const auto& sortedScores(l.getSortedScores(diffMult));
					ssvuj::Value response;

					unsigned int i{0};
					for(const auto& v : sortedScores)
					{
						ssvuj::set(response["r"][i], 0, v.second); ssvuj::set(response["r"][i], 1, v.first);
						++i;
						if(i > ssvu::getClamped(8u, 0u, static_cast<unsigned int>(sortedScores.size()))) break;
					}
					ssvuj::set(response, "id", levelId);

					float playerScore{l.getPlayerScore(username, diffMult)};
					playerScore == -1 ? ssvuj::set(response, "ps", "NULL") : ssvuj::set(response, "ps", playerScore);

					std::string leaderboardDataString;
					ssvuj::writeRootToString(response, leaderboardDataString);
					mMS.send(buildCompressedPacket<FromServer::SendLeaderboard>(leaderboardDataString));
				};

				pHandler[FromClient::RequestUserStats] = [&](ManagedSocket& mMS, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					ssvuj::Value statsValue;
					ssvuj::set(statsValue, users.getUser(username).stats);
					std::string response;
					ssvuj::writeRootToString(statsValue, response);
					mMS.send(buildCompressedPacket<FromServer::SendUserStats>(response));
				};

				pHandler[FromClient::US_Death] = [&](ManagedSocket&, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					users.getUser(username).stats.deaths += 1;
					saveUsers();
				};
				pHandler[FromClient::US_Restart] = [&](ManagedSocket&, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					users.getUser(username).stats.restarts += 1;
					saveUsers();
				};
				pHandler[FromClient::US_MinutePlayed] = [&](ManagedSocket&, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					users.getUser(username).stats.minutesSpentPlaying += 1;
					saveUsers();
				};
				pHandler[FromClient::US_AddFriend] = [&](ManagedSocket&, sf::Packet& mP)
				{
					ssvuj::Value request{getDecompressedPacket(mP)};
					std::string username{ssvuj::as<std::string>(request, 0)}, friendUsername{ssvuj::as<std::string>(request, 1)};
					if(username == friendUsername || !users.hasUser(friendUsername)) return;

					auto& tn(users.getUser(username).stats.trackedNames);
					if(ssvu::contains(tn, friendUsername)) return;
					tn.push_back(friendUsername);
					saveUsers();
				};
				pHandler[FromClient::US_ClearFriends] = [&](ManagedSocket&, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					users.getUser(username).stats.trackedNames.clear();
					saveUsers();
				};

				pHandler[FromClient::RequestFriendsScores] = [&](ManagedSocket& mMS, sf::Packet& mP)
				{
					ssvuj::Value request{getDecompressedPacket(mP)};
					std::string username{ssvuj::as<std::string>(request, 0)}, levelId{ssvuj::as<std::string>(request, 1)};

					if(!scores.hasLevel(levelId)) return;
					const auto& l(scores.getLevel(levelId));

					float diffMult{ssvuj::as<float>(request, 2)};
					ssvuj::Value responseValue;
					unsigned int i{0};

					for(const auto& n : users.getUser(username).stats.trackedNames)
					{
						const auto& score(l.getPlayerScore(n, diffMult));
						if(score == -1.f) continue;
						ssvuj::set(responseValue, i, score);
						++i;
						ssvuj::set(responseValue, i, l.getPlayerPosition(n, diffMult));
						++i;
					}

					std::string response; ssvuj::writeRootToString(responseValue, response);
					mMS.send(buildCompressedPacket<FromServer::SendFriendsScores>(response));
				};

				pHandler[FromClient::Logout] = [&](ManagedSocket& mMS, sf::Packet& mP)
				{
					ssvuj::Value request{getDecompressedPacket(mP)};
					std::string username{ssvuj::as<std::string>(request, 0)};
					if(!isLoggedIn(username)) return;
					logout(username);

					ssvu::lo << ssvu::lt("PacketHandler") << username << " logged out" << std::endl;
					mMS.send(buildPacket<FromServer::SendLogoutValid>());
				};
			}


			inline void start()
			{
				server.start(54000);
				while(true) std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		};
	}
}

#endif

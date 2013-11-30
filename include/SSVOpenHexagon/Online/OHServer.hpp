#ifndef HG_ONLINE_OHSERVER
#define HG_ONLINE_OHSERVER

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Online/PacketHandler.hpp"
#include "SSVOpenHexagon/Online/Server.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Online/Definitions.hpp"

namespace hg
{
	namespace Online
	{
		struct UserStats { unsigned int minutesSpentPlaying{0}, deaths{0}, restarts{0}; std::vector<std::string> trackedNames; };
		struct User	{ std::string passwordHash, email; UserStats stats; };
		class UserDB
		{
			template<typename T> friend struct ssvuj::Converter;

			private:
				std::unordered_map<std::string, User> users;

			public:
				inline bool hasUser(const std::string& mUsername) const						{ return users.count(mUsername) > 0; }
				inline User& getUser(const std::string& mUsername)							{ return users[mUsername]; }
				inline void registerUser(const std::string& mUsername, const User& mUser)	{ users[mUsername] = mUser; }
				inline const std::unordered_map<std::string, User>& getUsers() const		{ return users; }
				inline void setEmail(const std::string& mUsername, std::string mEmail)		{ users[mUsername].email = std::move(mEmail); }
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
					ss.emplace(mScore, mUsername);

					unsigned int i{1};
					for(auto ritr(std::rbegin(ss)); ritr != std::rend(ss); ++ritr) userPositions[mDiffMult][ritr->second] = i++;
				}

				inline bool hasDiffMult(float mDiffMult) const { return scores.count(mDiffMult) > 0; }
				inline const std::unordered_map<float, std::unordered_map<std::string, float>>& getScores() const { return scores; }
				inline const std::unordered_map<std::string, float>& getScores(float mDiffMult) const { return scores.at(mDiffMult); }
				inline const std::map<float, std::string>& getSortedScores(float mDiffMult) const { return sortedScores.at(mDiffMult); }
				inline float getPlayerScore(const std::string& mUsername, float mDiffMult) const { if(scores.count(mDiffMult) == 0 || scores.at(mDiffMult).count(mUsername) == 0) return -1.f; return scores.at(mDiffMult).at(mUsername); }
				inline int getPlayerPosition(const std::string& mUsername, float mDiffMult) const { if(userPositions.count(mDiffMult) == 0 || userPositions.at(mDiffMult).count(mUsername) == 0) return -1; return userPositions.at(mDiffMult).at(mUsername); }

		};
		class ScoreDB
		{
			template<typename T> friend struct ssvuj::Converter;

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
	template<> struct Converter<hg::Online::UserStats>
	{
		using T = hg::Online::UserStats;
		inline static void fromObj(T& mValue, const Obj& mObj)	{ extrObj(mObj, "dth", mValue.deaths, "msp", mValue.minutesSpentPlaying, "rst", mValue.restarts, "tn", mValue.trackedNames); }
		inline static void toObj(Obj& mObj, const T& mValue)	{ archObj(mObj, "dth", mValue.deaths, "msp", mValue.minutesSpentPlaying, "rst", mValue.restarts, "tn", mValue.trackedNames); }
	};
	template<> struct Converter<hg::Online::User>
	{
		using T = hg::Online::User;
		inline static void fromObj(T& mValue, const Obj& mObj)	{ extrObj(mObj, "ph", mValue.passwordHash, "em", mValue.email, "st", mValue.stats); }
		inline static void toObj(Obj& mObj, const T& mValue)	{ archObj(mObj, "ph", mValue.passwordHash, "em", mValue.email, "st", mValue.stats); }
	};
	template<> struct Converter<hg::Online::UserDB>
	{
		using T = hg::Online::UserDB;
		inline static void fromObj(T& mValue, const Obj& mObj)	{ extr(mObj, mValue.users); }
		inline static void toObj(Obj& mObj, const T& mValue)	{ arch(mObj, mValue.users); }
	};
	template<> struct Converter<hg::Online::LevelScoreDB>
	{
		using T = hg::Online::LevelScoreDB;
		inline static void fromObj(T& mValue, const Obj& mObj)
		{
			for(auto itr(std::begin(mObj)); itr != std::end(mObj); ++itr)
			{
				for(auto i(0u); i < size(*itr); ++i) mValue.addScore(std::stof(getKey(itr)), as<std::string>((*itr)[i], 0), as<float>((*itr)[i], 1));
			}
		}
		inline static void toObj(Obj& mObj, const T& mValue)
		{
			for(const auto& s : mValue.getScores())
			{
				auto i(0u);
				for(const auto& r : s.second)
				{
					Obj temp; set(temp, 0, r.first); set(temp, 1, r.second);
					set(mObj[ssvu::toStr(s.first)], i++, temp);
				}
			}
		}
	};
	template<> struct Converter<hg::Online::ScoreDB>
	{
		using T = hg::Online::ScoreDB;
		inline static void fromObj(T& mValue, const Obj& mObj)	{ extr(mObj, mValue.levels); }
		inline static void toObj(Obj& mObj, const T& mValue)	{ arch(mObj, mValue.levels); }
	};
}

namespace hg
{
	namespace Online
	{
		class LoginDB
		{
			private:
				ssvu::Bimap<unsigned int, std::string, std::unordered_map> logins;

			public:
				inline bool isLoggedIn(const std::string& mUsername) const					{ return logins.has(mUsername); }
				inline void acceptLogin(unsigned int mUid, const std::string& mUsername)	{ logins.insert({mUid, mUsername}); }

				inline void forceLogout(unsigned int mUid)			{ if(logins.has(mUid)) logins.erase(mUid); }
				inline void logout(const std::string& mUsername)	{ if(logins.has(mUsername)) logins.erase(mUsername); }
				inline std::vector<std::string> getLoggedUsernames() const { return ssvu::getKeys(logins.getMap2()); }
		};

		struct OHServer
		{
			ssvu::CommandLine::CmdLine cmdLine;

			bool verbose{true}, modifiedUsers{false}, modifiedScores{false};

			const std::string usersPath{"users.json"};
			const std::string scoresPath{"scores.json"};

			UserDB users{ssvuj::as<UserDB>(ssvuj::readFromFile(usersPath))};
			ScoreDB scores{ssvuj::as<ScoreDB>(ssvuj::readFromFile(scoresPath))};
			PacketHandler<ClientHandler> pHandler;
			Server server{pHandler};
			LoginDB loginDB; // currently logged-in users and uids

			std::future<void> inputFuture, saveFuture;

			inline void saveUsers()	const	{ ssvuj::Obj root; ssvuj::set(root, users); ssvuj::writeToFile(root, usersPath); }
			inline void saveScores() const	{ ssvuj::Obj root; ssvuj::set(root, scores); ssvuj::writeToFile(root, scoresPath); }
			inline User& getUserFromPacket(sf::Packet& mP) { return users.getUser(ssvuj::as<std::string>(getDecompressedPacket(mP), 0)); }

			OHServer()
			{
				ssvu::lo() << "OHServer constructed" << std::endl;

				server.onClientAccepted += [&](ClientHandler& mCH)
				{
					mCH.onDisconnect += [&]{ loginDB.forceLogout(mCH.getUid()); };
				};
				pHandler[FromClient::Ping] = [](ClientHandler&, sf::Packet&) { };
				pHandler[FromClient::Login] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					bool newUserRegistration{false};

					std::string username, password, passwordHash;
					ssvuj::extrArray(getDecompressedPacket(mP), username, password, passwordHash);

					if(loginDB.isLoggedIn(username))
					{
						if(verbose) ssvu::lo("PacketHandler") << "User already logged in" << std::endl;
						mMS.send(buildCPacket<FromServer::LoginResponseInvalid>()); return;
					}

					if(users.hasUser(username))
					{
						const auto& u(users.getUser(username));
						if(verbose) ssvu::lo("PacketHandler") << "Username found" << std::endl;

						if(u.passwordHash == passwordHash) ssvu::lo("PacketHandler") << "Password valid" << std::endl;
						else
						{
							if(verbose) ssvu::lo("PacketHandler") << "Password invalid" << std::endl;
							mMS.send(buildCPacket<FromServer::LoginResponseInvalid>()); return;
						}
					}
					else
					{
						if(verbose) ssvu::lo("PacketHandler") << "Username not found, registering" << std::endl;
						User newUser; newUser.passwordHash = passwordHash;
						users.registerUser(username, newUser); modifiedUsers = true;
						newUserRegistration = true;
					}

					if(verbose) ssvu::lo("PacketHandler") << "Accepting user" << std::endl;
					loginDB.acceptLogin(mMS.getUid(), username);
					mMS.send(buildCPacket<FromServer::LoginResponseValid>(newUserRegistration));
				};
				pHandler[FromClient::RequestInfo] = [](ClientHandler& mMS, sf::Packet&)
				{
					float version{2.f}; std::string message{"Welcome to Open Hexagon 2.0!"};
					mMS.send(buildCPacket<FromServer::RequestInfoResponse>(version, message));
				};
				pHandler[FromClient::SendScore] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username, levelId, validator; float diffMult, score;
					ssvuj::extrArray(getDecompressedPacket(mP), username, levelId, validator, diffMult, score);

					if(!loginDB.isLoggedIn(username)) { mMS.send(buildCPacket<FromServer::SendScoreResponseInvalid>()); return; }

					if(!scores.hasLevel(levelId))
					{
						if(verbose) ssvu::lo("PacketHandler") << "No table for this level id, creating one" << std::endl;
						scores.addLevel(levelId, {});
					}

					if(Online::getValidators().getValidator(levelId) != validator)
					{
						if(verbose) ssvu::lo("PacketHandler") << "Validator mismatch!\n" << Online::getValidators().getValidator(levelId) << "\n" << validator << std::endl;
						mMS.send(buildCPacket<FromServer::SendScoreResponseInvalid>()); return;
					}

					if(verbose) ssvu::lo("PacketHandler") << "Validator matches, inserting score" << std::endl;
					auto& l(scores.getLevel(levelId));
					if(l.getPlayerScore(username, diffMult) < score) { l.addScore(diffMult, username, score); modifiedScores = true; }
					mMS.send(buildCPacket<FromServer::SendScoreResponseValid>());
				};
				pHandler[FromClient::RequestLeaderboard] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username, levelId, validator; float diffMult;
					ssvuj::extrArray(getDecompressedPacket(mP), username, levelId, validator, diffMult);

					if(!loginDB.isLoggedIn(username))
					{
						if(verbose) ssvu::lo("PacketHandler") << "User not logged in!" << std::endl;
						mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return;
					}

					if(!scores.hasLevel(levelId)) { mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return; }
					auto& l(scores.getLevel(levelId));

					if(Online::getValidators().getValidator(levelId) != validator)
					{
						if(verbose) ssvu::lo("PacketHandler") << "Validator mismatch!\n" << Online::getValidators().getValidator(levelId) << "\n" << validator << std::endl;
						mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return;
					}

					if(!l.hasDiffMult(diffMult))
					{
						if(verbose) ssvu::lo("PacketHandler") << "No difficulty multiplier table!" << std::endl;
						mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return;
					}

					if(verbose) ssvu::lo("PacketHandler") << "Validator matches, sending leaderboard" << std::endl;

					const auto& sortedScores(l.getSortedScores(diffMult));
					ssvuj::Obj response;

					auto i(0u);
					for(auto itr(std::rbegin(sortedScores)); itr != std::rend(sortedScores); ++itr)
					{
						const auto& v(*itr);
						ssvuj::set(response["r"][i], 0, v.second); ssvuj::set(response["r"][i], 1, v.first);
						++i;
						if(i > ssvu::getClamped(8u, 0u, static_cast<unsigned int>(sortedScores.size()))) break;
					}
					ssvuj::set(response, "id", levelId);

					float playerScore{l.getPlayerScore(username, diffMult)}; playerScore == -1 ? ssvuj::set(response, "ps", "NULL") : ssvuj::set(response, "ps", playerScore);
					auto playerPosition(l.getPlayerPosition(username, diffMult)); playerPosition == -1 ? ssvuj::set(response, "pp", "NULL") : ssvuj::set(response, "pp", playerPosition);

					mMS.send(buildCPacket<FromServer::SendLeaderboard>(ssvuj::getWriteToString(response)));
				};
				pHandler[FromClient::NUR_Email] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username, email;
					ssvuj::extrArray(getDecompressedPacket(mP), username, email);

					users.setEmail(username, email);

					if(verbose)  ssvu::lo("PacketHandler") << "Email accepted" << std::endl;
					mMS.send(buildCPacket<FromServer::NUR_EmailValid>());
					modifiedUsers = true;
				};

				pHandler[FromClient::RequestUserStats] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					ssvuj::Obj response; ssvuj::set(response, users.getUser(username).stats);
					mMS.send(buildCPacket<FromServer::SendUserStats>(ssvuj::getWriteToString(response)));
				};


				// User statistics
				pHandler[FromClient::US_Death] = [&](ClientHandler&, sf::Packet& mP)		{ getUserFromPacket(mP).stats.deaths += 1; modifiedUsers = true; };
				pHandler[FromClient::US_Restart] = [&](ClientHandler&, sf::Packet& mP)		{ getUserFromPacket(mP).stats.restarts += 1; modifiedUsers = true; };
				pHandler[FromClient::US_MinutePlayed] = [&](ClientHandler&, sf::Packet& mP)	{ getUserFromPacket(mP).stats.minutesSpentPlaying += 1; modifiedUsers = true; };
				pHandler[FromClient::US_ClearFriends] = [&](ClientHandler&, sf::Packet& mP)	{ getUserFromPacket(mP).stats.trackedNames.clear(); modifiedUsers = true; };

				pHandler[FromClient::US_AddFriend] = [&](ClientHandler&, sf::Packet& mP)
				{
					std::string username, friendUsername;
					ssvuj::extrArray(getDecompressedPacket(mP), username, friendUsername);

					if(username == friendUsername || !users.hasUser(friendUsername)) return;

					auto& tn(users.getUser(username).stats.trackedNames);
					if(ssvu::contains(tn, friendUsername)) return;
					tn.push_back(friendUsername); modifiedUsers = true;
				};

				pHandler[FromClient::RequestFriendsScores] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username, levelId; float diffMult;
					ssvuj::extrArray(getDecompressedPacket(mP), username, levelId, diffMult);

					if(!scores.hasLevel(levelId)) return;
					const auto& l(scores.getLevel(levelId));

					ssvuj::Obj response;

					for(const auto& n : users.getUser(username).stats.trackedNames)
					{
						const auto& score(l.getPlayerScore(n, diffMult));
						if(score == -1.f) continue;
						ssvuj::set(response[n], 0, score);
						ssvuj::set(response[n], 1, l.getPlayerPosition(n, diffMult));
					}

					mMS.send(buildCPacket<FromServer::SendFriendsScores>(ssvuj::getWriteToString(response)));
				};

				pHandler[FromClient::Logout] = [&](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username{ssvuj::as<std::string>(getDecompressedPacket(mP), 0)};
					if(!loginDB.isLoggedIn(username)) return;
					loginDB.logout(username);
					if(verbose) ssvu::lo("PacketHandler") << username << " logged out" << std::endl;
					mMS.send(buildCPacket<FromServer::SendLogoutValid>());
				};
			}
			~OHServer() { saveIfNeeded(); ssvu::lo() << "OHServer destroyed" << std::endl; }

			inline void saveIfNeeded()
			{
				if(modifiedScores)
				{
					saveScores(); modifiedScores = false;
					if(verbose) ssvu::lo("saveIfNeeded") << "Saving scores..." << std::endl;
				}
				if(modifiedUsers)
				{
					saveUsers(); modifiedUsers = false;
					if(verbose) ssvu::lo("saveIfNeeded") << "Saving users..." << std::endl;
				}
			}

			inline void start()
			{
				//server.start(54000);
				server.start(27273);

				inputFuture = std::async(std::launch::async, [this]
				{
					while(server.isRunning())
					{
						std::string input;
						try { if(std::getline(std::cin, input)) cmdLine.parseCmdLine(ssvu::getSplit(input, ' ')); }
						catch(const std::runtime_error& mException) { ssvu::lo("CommandLine") << mException.what() << std::endl; }
						catch(...) { }
					}
				});

				saveFuture = std::async(std::launch::async, [this]{ while(server.isRunning()) { std::this_thread::sleep_for(std::chrono::seconds(3)); saveIfNeeded(); }});

				initCommands();

				// This loop keeps the server alive
				while(server.isRunning()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
			}

			void initCommands()
			{
				initCmdHelp();

				// Exit
				{
					auto& cmd(cmdLine.create({"exit", "quit", "close", "abort"}));
					cmd.setDesc("Stops the server.");
					cmd += [&]
					{
						ssvu::lo() << "Stopping server... saving if needed" << std::endl;
						saveIfNeeded();
						server.stop();
					};
				}

				// Toggle verbosity
				{
					auto& cmd(cmdLine.create({"verbose", "verbosity"}));
					cmd.setDesc("Sets log verbosity.");

					auto& arg(cmd.createArg<bool>());
					arg.setName("Enable verbosity?");
					arg.setBriefDesc("Controls whether verbosity is enabled or not.");

					cmd += [&]
					{
						verbose = arg.get();
						ssvu::lo("Verbose mode") << (verbose ? "on" : "off") << std::endl;
					};
				}

				// Data printing
				{
					auto& cmd(cmdLine.create({"log", "print", "show"}));
					cmd.setDesc("Logs current server data.");

					auto& arg(cmd.createArg<std::string>());
					arg.setName("What to print?");
					arg.setBriefDesc("Possible values: 'users', 'logins'.");

					cmd += [&]
					{
						if(arg.get() == "users")		{ for(const auto& u : users.getUsers()) ssvu::lo() << u.first << std::endl; }
						else if(arg.get() == "logins")	{ for(const auto& l : loginDB.getLoggedUsernames()) ssvu::lo() << l << std::endl; }
					};
				}
			}

			std::string getBriefHelp(const ssvu::CommandLine::Cmd& mCmd) { return mCmd.getNamesStr() + " " + mCmd.getArgsStr() + " " + mCmd.getOptArgsStr() + " " + mCmd.getFlagsStr() + " " + mCmd.getArgPacksStr(); }
			void initCmdHelp()
			{
				auto& cmd(cmdLine.create({"?", "help"}));
				cmd.setDesc("Show help for all commands or a single command.");

				auto& optArg(cmd.createOptArg<std::string>(""));
				optArg.setName("Command name");
				optArg.setBriefDesc("Name of the command to get help for.");
				optArg.setDesc("Leave blank to get general help.");

				auto& flagVerbose(cmd.createFlag("v", "verbose"));
				flagVerbose.setBriefDesc("Verbose general help?");

				cmd += [&]
				{
					if(!optArg)
					{
						ssvu::lo("Open Hexagon server help") << "\n\n";
						for(const auto& c : cmdLine.getCmds()) ssvu::lo() << getBriefHelp(*c) << "\n" << (flagVerbose ? c->getHelpStr() : "") << std::endl;
					}
					else
					{
						auto& c(cmdLine.findCmd(optArg.get()));
						ssvu::lo() << "\n" << getBriefHelp(c) << "\n" << c.getHelpStr();
						ssvu::lo().flush();
					}
				};
			}
		};
	}
}

#endif

#ifndef HG_ONLINE_OHSERVER
#define HG_ONLINE_OHSERVER

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Online/PacketHandler.hpp"
#include "SSVOpenHexagon/Online/Server.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Online/Definitions.hpp"
#include "SSVOpenHexagon/Online/Utils.hpp"

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

					int i{1};
					for(const auto& p : ssvu::asRangeReverse(ss)) userPositions[mDiffMult][p.second] = i++;
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
	template<> SSVUJ_CNV_SIMPLE(hg::Online::UserStats, mObj, mValue)	{ ssvuj::convertObj(mObj, "dth", mValue.deaths, "msp", mValue.minutesSpentPlaying, "rst", mValue.restarts, "tn", mValue.trackedNames); } SSVUJ_CNV_SIMPLE_END();
	template<> SSVUJ_CNV_SIMPLE(hg::Online::User, mObj, mValue)			{ ssvuj::convertObj(mObj, "ph", mValue.passwordHash, "em", mValue.email, "st", mValue.stats); } SSVUJ_CNV_SIMPLE_END();
	template<> SSVUJ_CNV_SIMPLE(hg::Online::UserDB, mObj, mValue)		{ ssvuj::convert(mObj, mValue.users); } SSVUJ_CNV_SIMPLE_END();
	template<> SSVUJ_CNV_SIMPLE(hg::Online::ScoreDB, mObj, mValue)		{ ssvuj::convert(mObj, mValue.levels); } SSVUJ_CNV_SIMPLE_END();

	template<> struct Converter<hg::Online::LevelScoreDB>
	{
		using T = hg::Online::LevelScoreDB;
		inline static void fromObj(const Obj& mObj, T& mValue)
		{
			for(auto itr(std::begin(mObj)); itr != std::end(mObj); ++itr)
			{
				for(auto i(0u); i < getObjSize(*itr); ++i) mValue.addScore(std::stof(getKey(itr)), getExtr<std::string>((*itr)[i], 0), getExtr<float>((*itr)[i], 1));
			}
		}
		inline static void toObj(Obj& mObj, const T& mValue)
		{
			for(const auto& s : mValue.getScores())
			{
				auto i(0u);
				for(const auto& r : s.second)
				{
					Obj temp; arch(temp, 0, r.first); arch(temp, 1, r.second);
					arch(mObj[ssvu::toStr(s.first)], i++, temp);
				}
			}
		}
	};
}

namespace hg
{
	namespace Online
	{
		class LoginDB
		{
			private:
				ssvu::Bimap<std::string, unsigned int> logins;

			public:
				inline bool isLoggedIn(const std::string& mUsername) const					{ return logins.has(mUsername); }
				inline void acceptLogin(unsigned int mUid, const std::string& mUsername)	{ logins.emplace(mUsername, mUid); }

				inline void forceLogout(unsigned int mUid)			{ if(logins.has(mUid)) logins.erase(mUid); }
				inline void logout(const std::string& mUsername)	{ if(logins.has(mUsername)) logins.erase(mUsername); }

				inline std::vector<std::string> getLoggedUsernames() const
				{
					std::vector<std::string> result;
					for(const auto& p : logins) result.emplace_back(p->first);
					return result;
				}
		};

		struct OHServer
		{
			ssvucl::Ctx ctx;

			bool modifiedUsers{false}, modifiedScores{false};

			const std::string usersPath{"users.json"};
			const std::string scoresPath{"scores.json"};

			UserDB users{ssvuj::getExtr<UserDB>(ssvuj::getFromFile(usersPath))};
			ScoreDB scores{ssvuj::getExtr<ScoreDB>(ssvuj::getFromFile(scoresPath))};
			PacketHandler<ClientHandler> pHandler;
			Server server{pHandler};
			LoginDB loginDB; // currently logged-in users and uids

			std::future<void> inputFuture, saveFuture;

			inline void saveUsers()	const	{ ssvuj::Obj root; ssvuj::arch(root, users); ssvuj::writeToFile(root, usersPath); }
			inline void saveScores() const	{ ssvuj::Obj root; ssvuj::arch(root, scores); ssvuj::writeToFile(root, scoresPath); }
			inline User& getUserFromPacket(sf::Packet& mP) { return users.getUser(ssvuj::getExtr<std::string>(getDecompressedPacket(mP), 0)); }

			OHServer()
			{
				ssvu::lo() << "OHServer constructed\n";

				server.onClientAccepted += [this](ClientHandler& mCH)
				{
					mCH.onDisconnect += [this, &mCH]{ loginDB.forceLogout(mCH.getUid()); };
				};
				pHandler[FromClient::Ping] = [](ClientHandler&, sf::Packet&) { };
				pHandler[FromClient::Login] = [this](ClientHandler& mMS, sf::Packet& mP)
				{
					bool newUserRegistration{false};

					std::string username, password, passwordHash;
					ssvuj::extrArray(getDecompressedPacket(mP), username, password, passwordHash);

					if(loginDB.isLoggedIn(username))
					{
						HG_LO_VERBOSE("PacketHandler") << "User already logged in\n";
						mMS.send(buildCPacket<FromServer::LoginResponseInvalid>()); return;
					}

					if(users.hasUser(username))
					{
						const auto& u(users.getUser(username));
						HG_LO_VERBOSE("PacketHandler") << "Username found\n";

						if(u.passwordHash == passwordHash)
						{
							HG_LO_VERBOSE("PacketHandler") << "Password valid\n";
						}
						else
						{
							HG_LO_VERBOSE("PacketHandler") << "Password invalid\n";
							mMS.send(buildCPacket<FromServer::LoginResponseInvalid>()); return;
						}
					}
					else
					{
						HG_LO_VERBOSE("PacketHandler") << "Username not found, registering\n";
						User newUser; newUser.passwordHash = passwordHash;
						users.registerUser(username, newUser); modifiedUsers = true;
						newUserRegistration = true;
					}

					HG_LO_VERBOSE("PacketHandler") << "Accepting user\n";
					loginDB.acceptLogin(mMS.getUid(), username);
					mMS.send(buildCPacket<FromServer::LoginResponseValid>(newUserRegistration));
				};
				pHandler[FromClient::RequestInfo] = [](ClientHandler& mMS, sf::Packet&)
				{
					float version{2.f}; std::string message{"Welcome to Open Hexagon 2.0!"};
					mMS.send(buildCPacket<FromServer::RequestInfoResponse>(version, message));
				};
				pHandler[FromClient::SendScore] = [this](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username, levelId, validator; float diffMult, score;
					ssvuj::extrArray(getDecompressedPacket(mP), username, levelId, validator, diffMult, score);

					if(!loginDB.isLoggedIn(username)) { mMS.send(buildCPacket<FromServer::SendScoreResponseInvalid>()); return; }

					if(!scores.hasLevel(levelId))
					{
						HG_LO_VERBOSE("PacketHandler") << "No table for this level id, creating one\n";
						scores.addLevel(levelId, {});
					}

					if(Online::getValidators().getValidator(levelId) != validator)
					{
						HG_LO_VERBOSE("PacketHandler") << "Validator mismatch!\n" << Online::getValidators().getValidator(levelId) << "\n" << validator << std::endl;
						mMS.send(buildCPacket<FromServer::SendScoreResponseInvalid>()); return;
					}

					HG_LO_VERBOSE("PacketHandler") << "Validator matches, inserting score\n";
					auto& l(scores.getLevel(levelId));
					if(l.getPlayerScore(username, diffMult) < score) { l.addScore(diffMult, username, score); modifiedScores = true; }
					mMS.send(buildCPacket<FromServer::SendScoreResponseValid>());
				};
				pHandler[FromClient::RequestLeaderboard] = [this](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username, levelId, validator; float diffMult;
					ssvuj::extrArray(getDecompressedPacket(mP), username, levelId, validator, diffMult);

					if(!loginDB.isLoggedIn(username))
					{
						HG_LO_VERBOSE("PacketHandler") << "User not logged in!\n";
						mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return;
					}

					if(!scores.hasLevel(levelId)) { mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return; }
					auto& l(scores.getLevel(levelId));

					if(Online::getValidators().getValidator(levelId) != validator)
					{
						HG_LO_VERBOSE("PacketHandler") << "Validator mismatch!\n" << Online::getValidators().getValidator(levelId) << "\n" << validator << std::endl;
						mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return;
					}

					if(!l.hasDiffMult(diffMult))
					{
						HG_LO_VERBOSE("PacketHandler") << "No difficulty multiplier table!\n";
						mMS.send(buildCPacket<FromServer::SendLeaderboardFailed>()); return;
					}

					HG_LO_VERBOSE("PacketHandler") << "Validator matches, sending leaderboard\n";

					const auto& sortedScores(l.getSortedScores(diffMult));
					ssvuj::Obj response;

					auto i(0u);
					for(const auto& v : ssvu::asRangeReverse(sortedScores))
					{
						auto& responseObj(ssvuj::getObj(response, "r"));
						auto& arrayObj(ssvuj::getObj(responseObj, i));

						ssvuj::arch(arrayObj, 0, v.second); ssvuj::arch(arrayObj, 1, v.first);
						++i;
						if(i > ssvu::getClamped(8u, 0u, static_cast<unsigned int>(sortedScores.size()))) break;
					}
					ssvuj::arch(response, "id", levelId);

					float playerScore{l.getPlayerScore(username, diffMult)};
					ssvuj::arch(response, "ps", playerScore);

					int playerPosition(l.getPlayerPosition(username, diffMult));
					ssvuj::arch(response, "pp", playerPosition);

					auto responseStr(ssvuj::getWriteToString(response));
					mMS.send(buildCPacket<FromServer::SendLeaderboard>(responseStr));
				};
				pHandler[FromClient::NUR_Email] = [this](ClientHandler& mMS, sf::Packet& mP)
				{
					ssvu::lo("PacketHandler") << "Received email packet\n";
					HG_LO_VERBOSE("PacketHandler") << "Received email packet\n";
					std::string username, email;
					ssvuj::extrArray(getDecompressedPacket(mP), username, email);

					users.setEmail(username, email);

					HG_LO_VERBOSE("PacketHandler") << "Email accepted\n";
					mMS.send(buildCPacket<FromServer::NUR_EmailValid>());
					modifiedUsers = true;
				};

				pHandler[FromClient::RequestUserStats] = [this](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username{ssvuj::getExtr<std::string>(getDecompressedPacket(mP), 0)};
					ssvuj::Obj response; ssvuj::arch(response, users.getUser(username).stats);
					mMS.send(buildCPacket<FromServer::SendUserStats>(ssvuj::getWriteToString(response)));
				};


				// User statistics
				pHandler[FromClient::US_Death] = [this](ClientHandler&, sf::Packet& mP)			{ getUserFromPacket(mP).stats.deaths += 1; modifiedUsers = true; };
				pHandler[FromClient::US_Restart] = [this](ClientHandler&, sf::Packet& mP)		{ getUserFromPacket(mP).stats.restarts += 1; modifiedUsers = true; };
				pHandler[FromClient::US_MinutePlayed] = [this](ClientHandler&, sf::Packet& mP)	{ getUserFromPacket(mP).stats.minutesSpentPlaying += 1; modifiedUsers = true; };
				pHandler[FromClient::US_ClearFriends] = [this](ClientHandler&, sf::Packet& mP)	{ getUserFromPacket(mP).stats.trackedNames.clear(); modifiedUsers = true; };

				pHandler[FromClient::US_AddFriend] = [this](ClientHandler&, sf::Packet& mP)
				{
					std::string username, friendUsername;
					ssvuj::extrArray(getDecompressedPacket(mP), username, friendUsername);

					if(username == friendUsername || !users.hasUser(friendUsername)) return;

					auto& tn(users.getUser(username).stats.trackedNames);
					if(ssvu::contains(tn, friendUsername)) return;
					tn.emplace_back(friendUsername); modifiedUsers = true;
				};

				pHandler[FromClient::RequestFriendsScores] = [this](ClientHandler& mMS, sf::Packet& mP)
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
						ssvuj::arch(response[n], 0, score);
						ssvuj::arch(response[n], 1, l.getPlayerPosition(n, diffMult));
					}

					mMS.send(buildCPacket<FromServer::SendFriendsScores>(ssvuj::getWriteToString(response)));
				};

				pHandler[FromClient::Logout] = [this](ClientHandler& mMS, sf::Packet& mP)
				{
					std::string username{ssvuj::getExtr<std::string>(getDecompressedPacket(mP), 0)};
					if(!loginDB.isLoggedIn(username)) return;
					loginDB.logout(username);
					HG_LO_VERBOSE("PacketHandler") << username << " logged out\n";
					mMS.send(buildCPacket<FromServer::SendLogoutValid>());
				};
			}
			~OHServer() { saveIfNeeded(); ssvu::lo() << "OHServer destroyed\n"; }

			inline void saveIfNeeded()
			{
				if(modifiedScores)
				{
					saveScores(); modifiedScores = false;
					HG_LO_VERBOSE("saveIfNeeded") << "Saving scores...\n";
				}
				if(modifiedUsers)
				{
					saveUsers(); modifiedUsers = false;
					HG_LO_VERBOSE("saveIfNeeded") << "Saving users...\n";
				}
			}

			inline void start()
			{
				server.start(Online::getCurrentPort());

				inputFuture = std::async(std::launch::async, [this]
				{
					while(server.isRunning())
					{
						ssvu::lo().flush();
						std::string input;

						try
						{
							if(std::getline(std::cin, input)) ctx.process(ssvu::getSplit(input, ' '));
						}
						catch(const ssvucl::Exception::Base& mEx)
						{
							ssvu::lo(mEx.getTitle()) << mEx.what() << std::endl;
						}
						catch(const std::runtime_error& mEx)
						{
							ssvu::lo("Runtime error") << mEx.what() << std::endl;
						}
						catch(...)
						{

						}
					}
				});

				saveFuture = std::async(std::launch::async, [this]{ while(server.isRunning()) { std::this_thread::sleep_for(5s); saveIfNeeded(); }});

				initCommands();

				// This loop keeps the server alive
				while(server.isRunning()) { std::this_thread::sleep_for(10s); }
			}

			void initCommands()
			{
				initCmdHelp();

				// Exit
				{
					auto& cmd(ctx.create({"exit", "quit", "close", "abort"}));
					cmd.setDesc("Stops the server.");
					cmd += [this]
					{
						ssvu::lo() << "Stopping server... saving if needed\n";
						saveIfNeeded();
						server.stop();
					};
				}

				// Toggle verbosity
				{
					auto& cmd(ctx.create({"verbose", "verbosity"}));
					cmd.setDesc("Sets log verbosity.");

					auto& arg(cmd.create<ssvucl::Arg<bool>>());
					arg.setName("Enable verbosity?");
					arg.setBriefDesc("Controls whether verbosity is enabled or not.");

					cmd += [this, &arg]
					{
						Config::setServerVerbose(arg.get());
						ssvu::lo("Verbose mode") << (Config::getServerVerbose() ? "on" : "off") << std::endl;
					};
				}

				// Data printing
				{
					auto& cmd(ctx.create({"log", "print", "show"}));
					cmd.setDesc("Logs current server data.");

					auto& arg(cmd.create<ssvucl::Arg<std::string>>());
					arg.setName("What to print?");
					arg.setBriefDesc("Possible values: 'users', 'logins'.");

					cmd += [this, &arg]
					{
						if(arg.get() == "users")		{ for(const auto& u : users.getUsers()) ssvu::lo() << u.first << std::endl; }
						else if(arg.get() == "logins")	{ for(const auto& l : loginDB.getLoggedUsernames()) ssvu::lo() << l << std::endl; }
					};
				}
			}

			std::string getBriefHelp(const ssvucl::Cmd& mCmd)
			{
				using namespace ssvucl;
				return mCmd.getNamesStr() + " " + mCmd.getStr<EType::Arg>() + " " + mCmd.getStr<EType::ArgOpt>() + " " + mCmd.getStr<EType::Flag>() + " " + mCmd.getStr<EType::ArgPack>();
			}
			void initCmdHelp()
			{
				auto& cmd(ctx.create({"?", "help"}));
				cmd.setDesc("Show help for all commands or a single command.");

				auto& argOpt(cmd.create<ssvucl::ArgOpt<std::string>>(""));
				argOpt.setName("Command name");
				argOpt.setBriefDesc("Name of the command to get help for.");
				argOpt.setDesc("Leave blank to get general help.");

				auto& flagVerbose(cmd.create<ssvucl::Flag>("v", "verbose"));
				flagVerbose.setBriefDesc("Verbose general help?");

				cmd += [this, &argOpt, &flagVerbose]
				{
					if(!argOpt)
					{
						ssvu::lo("Open Hexagon server help") << "\n\n";
						for(const auto& c : ctx.getCmds()) ssvu::lo() << getBriefHelp(*c) << "\n" << (flagVerbose ? c->getHelpStr() : "") << std::endl;
					}
					else
					{
						auto& c(ctx.findCmd(argOpt.get()));
						ssvu::lo() << "\n" << getBriefHelp(c) << "\n" << c.getHelpStr();
						ssvu::lo().flush();
					}
				};
			}
		};
	}
}

#endif

// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ONLINE
#define HG_ONLINE

#include <string>
#include <future>
#include <unordered_map>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>
#include <SFML/Network.hpp>

namespace hg
{
	namespace Online
	{
		class User
		{
			private:
				std::string passwordHash;

			public:
				inline void setPasswordHash(const std::string& mPasswordHash) { passwordHash = mPasswordHash; }
				inline const std::string& getPasswordHash() const { return passwordHash; }
				inline bool isPasswordHash(const std::string& mPasswordHash) const { return passwordHash == mPasswordHash; }
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
				std::string validator;
				std::unordered_map<float, std::unordered_map<std::string, float>> scores;
				std::unordered_map<float, std::map<float, std::string>> sortedScores;

			public:
				inline void setValidator(const std::string& mValidator) { validator = mValidator; }
				inline void addScore(float mDiffMult, const std::string& mUsername, float mScore)
				{
					scores[mDiffMult][mUsername] = mScore;
					sortedScores[mDiffMult].insert({mScore, mUsername});
				}
				inline bool isValidator(const std::string& mValidator) const { return validator == mValidator; }
				inline bool hasDiffMult(float mDiffMult) const { return scores.count(mDiffMult) > 0; }
				inline float getScore(float mDiffMult, const std::string& mUsername) const { if(!hasDiffMult(mDiffMult) || scores.at(mDiffMult).count(mUsername) == 0) return -1; return scores.at(mDiffMult).at(mUsername); }
				inline const std::string& getValidator() const { return validator; }
				inline const std::unordered_map<float, std::unordered_map<std::string, float>>& getScores() const { return scores; }
				inline const std::unordered_map<std::string, float>& getScores(float mDiffMult) const { return scores.at(mDiffMult); }
				inline float getPlayerScore(const std::string& mUsername, float mDiffMult) const { for(const auto& v : getScores(mDiffMult)) if(v.first == mUsername) return v.second; return -1.f; }
				inline const std::map<float, std::string>& getSortedScores(float mDiffMult) const { return sortedScores.at(mDiffMult); }

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


		using Listener = sf::TcpListener;
		using Packet = sf::Packet;
		using Socket = sf::TcpSocket;
		using IpAddress = sf::IpAddress;

		void initializeServer();
		void initializeClient();

		void tryConnectToServer();
		bool isConnected();

		void tryLogin(const std::string& mUsername, const std::string& mPassword);
		bool isLoggedIn();

		void trySendScore(const std::string& mUsername, const std::string& mLevelId, const std::string& mValidator, float mDiffMult, float mScore);
		void tryRequestLeaderboard(const std::string& mUsername, const std::string& mLevelId, const std::string& mValidator, float mDiffMult);

		void invalidateCurrentLeaderboard();
		const std::string& getCurrentLeaderboard();

		float getServerVersion();
		std::string getServerMessage();
		std::string getMD5Hash(const std::string& mString);
		inline std::string getUrlEncoded(const std::string& mString) 		{ std::string result; for(const auto& c : mString) if(std::isalnum(c)) result += c; return result; }
		inline std::string getControlStripped(const std::string& mString)	{ std::string result; for(const auto& c : mString) if(!std::iscntrl(c)) result += c; return result; }

		enum class LogMode { Quiet, Verbose };

		// Client to server
		enum ClientPackets : unsigned int
		{
			Ping = 0,
			Login = 1,
			RequestInfo = 2,
			SendScore = 3,
			RequestLeaderboard = 4
		};

		// Server to client
		enum ServerPackets : unsigned int
		{
			LoginResponseValid = 0,
			LoginResponseInvalid = 1,
			RequestInfoResponse = 2,
			SendLeaderboard = 3,
			SendScoreResponseValid = 4,
			SendScoreResponseInvalid = 5,
			SendLeaderboardFailed = 6
		};

		template<int TTimes = 5, LogMode TLM = LogMode::Quiet> std::future<bool> retry(std::function<bool()> mFunc, const std::chrono::duration<int, std::milli>& mDuration = std::chrono::milliseconds(1500))
		{
			auto result(std::async(std::launch::async, [=]
			{
				for(int i{0}; i < TTimes; ++i)
				{
					if(mFunc()) return true;

					if(TLM == LogMode::Verbose) ssvu::lo << ssvu::lt("asyncTry") << "Error - retrying (" << i + 1 << "/" << TTimes << ")" << std::endl;
					std::this_thread::sleep_for(mDuration);
				}

				return false;
			}));

			return result;
		}
	}
}

namespace ssvuj
{
	namespace Internal
	{
		template<> struct AsHelper<hg::Online::User>
		{
			inline static hg::Online::User as(const Impl& mValue)
			{
				hg::Online::User result;
				result.setPasswordHash(ssvuj::as<std::string>(mValue, "ph"));
				return result;
			}
		};

		template<> struct AsHelper<hg::Online::UserDB>
		{
			inline static hg::Online::UserDB as(const Impl& mValue)
			{
				hg::Online::UserDB result;

				for(auto itr(std::begin(mValue)); itr != std::end(mValue); ++itr)
					result.registerUser(ssvuj::as<std::string>(itr.key()), ssvuj::as<hg::Online::User>(*itr));

				return result;
			}
		};

		template<> struct AsHelper<hg::Online::LevelScoreDB>
		{
			inline static hg::Online::LevelScoreDB as(const Impl& mValue)
			{
				hg::Online::LevelScoreDB result;

				result.setValidator(ssvuj::as<std::string>(mValue, "validator"));

				for(auto itr(std::begin(mValue)); itr != std::end(mValue); ++itr)
				{
					if(ssvuj::as<std::string>(itr.key()) == "validator") continue;


					for(unsigned int i{0}; i < ssvuj::size(*itr); ++i)
						result.addScore(std::stof(ssvuj::as<std::string>(itr.key())), ssvuj::as<std::string>((*itr)[i], 0), ssvuj::as<float>((*itr)[i], 1));


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

	template<> inline void set<hg::Online::User>(Impl& mRoot, const hg::Online::User& mValueToSet)
	{
		set(mRoot, "ph", mValueToSet.getPasswordHash());
	}

	template<> inline void set<hg::Online::UserDB>(Impl& mRoot, const hg::Online::UserDB& mValueToSet)
	{
		for(const auto p : mValueToSet.getUsers()) ssvuj::set(mRoot, p.first, p.second);
	}

	template<> inline void set<hg::Online::LevelScoreDB>(Impl& mRoot, const hg::Online::LevelScoreDB& mValueToSet)
	{
		ssvuj::set(mRoot, "validator", mValueToSet.getValidator());
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

#endif

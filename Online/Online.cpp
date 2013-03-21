// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <functional>
#include <json/reader.h>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>
#include "Online/Online.h"
#include "Global/Config.h"
#include "Online/Definitions.h"
#include "Utils/Utils.h"
#include "Utils/ThreadWrapper.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;
using namespace ssvu::FileSystem;

namespace hg
{
	namespace Online
	{
		using Request = Http::Request;
		using Response = Http::Response;
		using Status = Http::Response::Status;

		const string host{"http://vittorioromeo.info"};
		const string folder{"Misc/Linked/OHServer/"};
		const string infoFile{"OHInfo.json"};
		const string scoresFile{"scores.json"};
		const string sendScoreFile{"sendScore.php"};
		const string getScoresFile{"getScores.php"};

		MemoryManager<ThreadWrapper> memoryManager;
		float serverVersion{-1};
		string serverMessage{""};
		Json::Value scoresRoot;

		void startCheckUpdates()
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }

			ThreadWrapper& thread = memoryManager.create([]
			{
				log("Checking updates...", "Online");

				Response response{getGetResponse(host, folder, infoFile)};
				Status status{response.getStatus()};
				if(status == Response::Ok)
				{
					Json::Value root{getRootFromString(response.getBody())};
					serverMessage = getValueOrDefault<string>(root, "message", "");
					log("Server message:\n" + serverMessage, "Online");

					serverVersion = getValueOrDefault<float>(root, "latest_version", -1);
					log("Server latest version: " + toStr(getServerVersion()), "Online");

					if(serverVersion == getVersion()) log("No updates available", "Online");
					else if(serverVersion < getVersion()) log("Your version is newer than the server's (beta)", "Online");
					else if(serverVersion > getVersion()) log("Update available (" + toStr(serverVersion) + ")", "Online");
				}
				else
				{
					serverVersion = -1;
					serverMessage = "Error connecting to server";
					log("Error checking updates - code: " + status, "Online");
				}

				log("Finished checking updates", "Online");
				cleanUp();
			});

			thread.launch();
		}
		void startCheckScores()
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }

			ThreadWrapper& thread = memoryManager.create([]
			{
				log("Checking scores...", "Online");

				Response response{getGetResponse(host, folder, scoresFile)};
				Status status{response.getStatus()};
				if(status == Response::Ok)
				{
					Json::Reader reader; reader.parse(response.getBody(), scoresRoot);
					log("Scores retrieved successfully", "Online");
				}
				else log("Error checking scores - code: " + status, "Online");

				log("Finished checking scores", "Online");
				cleanUp();
			});

			thread.launch();
		}
		void startSendScore(const string& mName, const string& mValidator, float mScore)
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }

			ThreadWrapper& thread = memoryManager.create([=]
			{
				log("Sending score to server...", "Online");

				string scoreString{toStr(mScore)};
				string body{"n=" + mName + "&v=" + mValidator + "&s=" + scoreString + "&k=" + HG_ENCRYPTIONKEY};
				Response response{getPostResponse(host, folder, sendScoreFile, body)};
				Status status{response.getStatus()};

				if(status == Response::Ok) log("Score sent successfully: " + mName + ", " + scoreString, "Online");
				else log("Send score error: " + status, "Online");

				log("Finished sending score", "Online"); log(""); log(response.getBody(), "Server Message");
				startCheckScores();
				cleanUp();
			});

			ThreadWrapper& checkThread = memoryManager.create([&thread]
			{
				log("Checking if score can be sent...", "Online");

				while(serverVersion == -1)
				{
					log("Can't send score to server - version not checked, retrying...", "Online");
					sleep(seconds(5)); startCheckUpdates();
				}

				if(serverVersion > getVersion()) { log("Can't send score to server - version outdated", "Online"); return; }

				log("Score can be sent - sending", "Online");
				thread.launch();
				cleanUp();
			});

			checkThread.launch();
		}
		void startGetScores(string& mTargetString, const string& mValidator)
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }

			ThreadWrapper& thread = memoryManager.create([=, &mTargetString]
			{
				mTargetString = "";

				log("Getting scores from server...", "Online");

				string body{"v=" + mValidator};
				Response response{getPostResponse(host, folder, getScoresFile, body)};
				Status status{response.getStatus()};

				if(status == Response::Ok)
				{
					log("Scores got successfully", "Online");
					mTargetString = response.getBody();
				}
				else log("Get scores error: " + status, "Online");

				log("Finished getting scores", "Online");
				cleanUp();
			});

			thread.launch();
		}

		void cleanUp() 		{ for(auto& t : memoryManager.getItems()) if(t->getFinished()) memoryManager.del(t); memoryManager.cleanUp(); }
		void terminateAll() { for(auto& t : memoryManager.getItems()) t->terminate(); memoryManager.cleanUp(); }

		string getValidator(const string& mPackPath, const string& mLevelId, const string& mLevelRootPath,
			const string& mStyleRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt{""};
			toEncrypt.append(mLevelId);
			toEncrypt.append(toStr(mDifficultyMultiplier));
			toEncrypt.append(getFileContents(mLevelRootPath));
			toEncrypt.append(getFileContents(mStyleRootPath));
			toEncrypt.append(luaScriptContents);

			for(auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName};
				string contents{getFileContents(path)};
				toEncrypt.append(contents);
			}

			toEncrypt = getControlStripped(toEncrypt);

			string result{getUrlEncoded(mLevelId) + getMD5Hash(toEncrypt + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};
			return result;
		}

		float getServerVersion() 								{ return serverVersion; }
		string getServerMessage() 								{ return serverMessage; }
		Json::Value getScores(const std::string& mValidator) 	{ return scoresRoot[mValidator]; }
		string getMD5Hash(const string& mString) 				{ MD5 key{mString}; return key.GetHash(); }
		string getUrlEncoded(const string& mString) 			{ string result{""}; for(auto c : mString) if(isalnum(c)) result += c; return result; }
		string getControlStripped(const string& mString)		{ string result{""}; for(auto c : mString) if(!iscntrl(c)) result += c; return result; }
		
	}
}


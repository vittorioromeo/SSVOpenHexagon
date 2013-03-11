// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <functional>
#include <SFML/Network.hpp>
#include <json/json.h>
#include <json/reader.h>
#include <SSVStart.h>
#include "Online.h"
#include "Global/Config.h"
#include "Utils/Utils.h"
#include "Utils/MD5.h"
#include "Online/ThreadWrapper.h"
#include "Online/Definitions.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;

namespace hg
{
	namespace Online
	{
		using Request = Http::Request;
		using Response = Http::Response;
		using Status = Http::Response::Status;

		const string host{"http://vittorioromeo.info"};
		const string folder{"Misc/Linked/OHServer/"};

		MemoryManager<ThreadWrapper> memoryManager;
		float serverVersion{-1};
		string serverMessage{""};
		Json::Value scoresRoot;

		Response getResponse(const string& mRequestFile){ return Http(host).sendRequest({folder + mRequestFile}); }

		void startCheckUpdates()
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }

			ThreadWrapper& thread = memoryManager.create([]
			{
				log("Checking updates...", "Online");

				Response response{getResponse("OHInfo.json")};
				Status status{response.getStatus()};
				if(status == Response::Ok)
				{
					Json::Value root; Json::Reader reader; reader.parse(response.getBody(), root);

					string message(getJsonValueOrDefault<string>(root, "message", ""));
					serverMessage = message;
					log("Server message:\n" + message, "Online");

					serverVersion = (getJsonValueOrDefault<float>(root, "latest_version", -1));
					log("Server latest version: " + toStr(getServerVersion()), "Online");

					if(getServerVersion() == getVersion()) log("No updates available", "Online");
					else if(getServerVersion() < getVersion()) log("Your version is newer than the server's (beta)", "Online");
					else if(getServerVersion() > getVersion()) log("Update available (" + toStr(getServerVersion()) + ")", "Online");
				}
				else
				{
					serverVersion = -1;
					log("Error checking updates", "Online");
					log("Error code: " + status, "Online");
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

				Response response{getResponse("scores.json")};
				Status status{response.getStatus()};
				if(status == Response::Ok)
				{
					Json::Reader reader; reader.parse(response.getBody(), scoresRoot);
					log("Scores retrieved successfully", "Online");
				}
				else
				{
					log("Error checking scores", "Online");
					log("Error code: " + status, "Online");
				}

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
				string body{"n=" + mName + "&v=" + mValidator + "&s=" + scoreString + "&k=" + getMD5Hash(mName + mValidator + scoreString + HG_SERVER_KEY)};
				Http http(host); Request request(folder + "sendScore.php", Request::Post, body);
				Response response{http.sendRequest(request)};
				Status status{response.getStatus()};

				if(status == Response::Ok) log("Score sent successfully: " + mName + ", " + scoreString, "Online");
				else log("Send score error: " + status, "Online");

				log("Finished sending score", "Online");
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

		void cleanUp()
		{
			for(auto& thread : memoryManager.getItems()) if(thread->getFinished()) memoryManager.del(thread); 
			memoryManager.cleanUp();
		}
		void terminateAll()
		{
			for(auto& thread : memoryManager.getItems()) thread->terminate();
			memoryManager.cleanUp();
		}

		float getServerVersion() { return serverVersion; }
		string getServerMessage() { return serverMessage; }
		Json::Value getScores(const std::string& mValidator) { return scoresRoot[mValidator]; }
		string getMD5Hash(const string& mString) { MD5 key{mString}; return key.GetHash(); }
		string getUrlEncoded(const string& mString)
		{
			string result{""};
			for(unsigned int i{0}; i < mString.size(); ++i) if(isalnum(mString[i])) result += mString[i];
			return result;
		}
		string getValidator(const string& mPackPath, const string& mLevelId, const string& mJsonRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};

			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string result{""};
			result.append(getUrlEncoded(mLevelId));
			result.append(getMD5Hash(getFileContents(mJsonRootPath) + HG_SERVER_KEY));
			result.append(getMD5Hash(luaScriptContents + HG_SERVER_KEY));

			for(auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName};
				string contents{getFileContents(path)};
				string hash{getMD5Hash(contents + HG_SERVER_KEY)};
				string compressedHash{""};

				for(unsigned int i{0}; i < hash.length(); ++i) if(i % 3 == 0) compressedHash.append(toStr(hash[i]));

				result.append(compressedHash);
			}

			result.append(getUrlEncoded(toStr(mDifficultyMultiplier)));

			return result;
		}
	}
}


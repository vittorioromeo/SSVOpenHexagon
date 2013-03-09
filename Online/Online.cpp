#include <functional>
#include <SFML/Network.hpp>
#include <json/json.h>
#include <json/reader.h>
#include <SSVStart.h>
#include "Online.h"
#include "Global/Config.h"
#include "Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;

namespace hg
{
	namespace Online
	{
		struct ThreadWrapper
		{
			bool finished;
			function<void()> func;
			Thread thread;
			ThreadWrapper(function<void()> mFunction) : finished{false}, func{[&, mFunction]{ mFunction(); finished = true; }}, thread{func} { }
			void launch() { thread.launch(); }
			void terminate() { thread.terminate(); }
		};

		MemoryManager<ThreadWrapper> memoryManager;
		float serverVersion{-1};
		Json::Value scoresRoot;

		void startCheckUpdates()
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }

			ThreadWrapper& thread = memoryManager.create([]
			{
				log("Checking updates...", "Online");

				Http http; http.setHost("http://vittorioromeo.info");
				Http::Request request("Misc/Linked/OHServer/OHInfo.json");
				Http::Response response{http.sendRequest(request)};
				Http::Response::Status status{response.getStatus()};
				if(status == Http::Response::Ok)
				{
					Json::Value root; Json::Reader reader; reader.parse(response.getBody(), root);

					string message(getJsonValueOrDefault<string>(root, "message", ""));
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

				Http http; http.setHost("http://vittorioromeo.info");
				Http::Request request("Misc/Linked/OHServer/scores.json");
				Http::Response response{http.sendRequest(request)};
				Http::Response::Status status{response.getStatus()};
				if(status == Http::Response::Ok)
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

				Http http; http.setHost("http://vittorioromeo.info");
				string args{"n=" + mName + "&v=" + mValidator + "&s=" + toStr(mScore)};
				Http::Request request("Misc/Linked/OHServer/sendScore.php", Http::Request::Post); request.setBody(args);
				Http::Response response{http.sendRequest(request)};
				Http::Response::Status status{response.getStatus()};

				if(status == Http::Response::Ok) log("Score sent successfully", "Online");
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
			for(auto& thread : memoryManager.getItems()) if(thread->finished) memoryManager.del(thread); 
			memoryManager.cleanUp();
		}
		void terminateAll()
		{
			for(auto& thread : memoryManager.getItems()) thread->terminate();
			memoryManager.cleanUp();
		}

		float getServerVersion() { return serverVersion; }
		Json::Value getScores(const std::string& mValidator) { return scoresRoot[mValidator]; }
		string getValidator(const string& mLevelId, const string& mJsonRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string result{""};
			result.append(getStripped(mLevelId));
			result.append(getCompressed(getStripped(getFileContents(mJsonRootPath))));
			result.append(getCompressed(getStripped(getFileContents(mLuaScriptPath))));
			result.append(toStr(mDifficultyMultiplier));
			return result;
		}
		string getStripped(const string& mString)
		{
			string result{mString};
			vector<string> toStrip{" ", "\n", "\t", "\v", "\f", "\r", "\\", "/", "\"", "&", "?", "{", "}", "[", "]", "(", ")", "=", ",", ":", "-", "_", ".", "!"};
			for(auto& s : toStrip) result = replaceAll(result, s, "");
			return result;
		}
		string getCompressed(const string& mString)
		{
			string result{""};
			for(unsigned int i{0}; i < mString.size(); ++i) if(i % 15 == 0) result += mString[i];
			return result;
		}
	}
}


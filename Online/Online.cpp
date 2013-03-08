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
		MemoryManager<Thread> memoryManager;
		vector<string> checkedValidators;
		map<string, bool> scoresBeingChecked;
		MemoryManager<pair<string, vector<pair<string, float>>>> vecMemoryManager;
		bool updatesChecked{false};
		float serverVersion{-1};

		void checkUpdates()
		{
			memoryManager.create([]
			{
				Http http;
				http.setHost("http://vittorioromeo.info");
				Http::Request request("Misc/Linked/OHServer/OHInfo.json");
				Http::Response response{http.sendRequest(request)};
				Http::Response::Status status{response.getStatus()};
				if(status == Http::Response::Ok)
				{
					Json::Value root; Json::Reader reader; reader.parse(response.getBody(), root);

					string message(getJsonValueOrDefault<string>(root, "message", ""));
					log("Server message:\n" + message, "Online");

					setServerVersion(getJsonValueOrDefault<float>(root, "latest_version", -1));
					log("Server latest version: " + toStr(getServerVersion()), "Online");

					if(getServerVersion() == getVersion()) log("No updates available", "Online");
					else if(getServerVersion() < getVersion()) log("Your version is newer than the server's (beta)", "Online");
					else if(getServerVersion() > getVersion()) log("Update available (" + toStr(getServerVersion()) + ")", "Online");
				}
				else
				{
					setServerVersion(-1);
					log("Error connecting to the server", "Online");
				}

				setUpdatesChecked(true);
			}).launch();
		}

		void sendScore(const string& mProfileName, const string& mLevelValidator, float mScore)
		{
			string compressedValidator{getCompressed(mLevelValidator)};

			Http http;
			http.setHost("http://vittorioromeo.info");
			string phpArguments{"profileName=" + mProfileName + "&levelValidator=" + compressedValidator + "&score=" + toStr(mScore)};


			string data{"Content-Type: multipart/form-data;\n\nContent-Disposition: form-data; name=\"profileName\"\nContent-Type: text/plain\n\n" +
							mProfileName + "\n\nContent-Disposition: form-data; name=\"levelValidator\"\nContent-Type: text/plain\n\n" +
							mLevelValidator + "\n\nContent-Disposition: form-data; name=\"score\"\nContent-Type: text/plain\n\n" + toStr(mScore)};
cout <<phpArguments << endl;

			Http::Request request("Misc/Linked/OHServer/sendScore.php", Http::Request::Post);
			request.setBody(phpArguments);
			Http::Response response{http.sendRequest(request)};
			Http::Response::Status status{response.getStatus()};

			log(response.getStatus(), "Online");


			return;

			auto& thread = memoryManager.create([=]
			{
				
			});

			auto& checkThread = memoryManager.create([=, &thread]
			{
				while(!updatesChecked)
				{
					if(getServerVersion() > getVersion())
					{
						log("Can't send score to server - version outdates", "Online");
						return;
					}
					else
					{
						log("Can't send score to server - version not checked, retrying...", "Online");
						sleep(seconds(5));
						checkUpdates();
					}
				}

				thread.launch();
			});

			checkThread.launch();
		}
		vector<pair<string, float>>& getScores(const string& mLevelValidator)
		{
			vector<pair<string, float>>* result{nullptr};

			if(!contains(checkedValidators, mLevelValidator))
			{
				checkedValidators.push_back(mLevelValidator);
				result = &(vecMemoryManager.create(mLevelValidator, vector<pair<string, float>>{}).second);
			}
			else
			{
				for(auto& pair : vecMemoryManager.getItems()) if(pair->first == mLevelValidator) { result = &(pair->second); break; }
			}

			if(scoresBeingChecked.find(mLevelValidator) == scoresBeingChecked.end()) scoresBeingChecked[mLevelValidator] = false;
			if(scoresBeingChecked[mLevelValidator]) return *result;
			scoresBeingChecked[mLevelValidator] = true;

			memoryManager.create([result, mLevelValidator, &scoresBeingChecked]
			{
				log("Checking scores", "Online");
				
				Http http;
				http.setHost("http://vittorioromeo.info");
				Http::Request request("Misc/Linked/OHServer/scores.json");
				Http::Response response{http.sendRequest(request)};
				Http::Response::Status status{response.getStatus()};
				if(status == Http::Response::Ok)
				{
					Json::Value root; Json::Reader reader; reader.parse(response.getBody(), root);
					Json::Value array{root[mLevelValidator]};
					for(unsigned int i{0}; i < root["mLevelValidator"].size(); ++i)
						result->push_back({root[mLevelValidator][i]["profileName"].asString(), root[mLevelValidator][i]["score"].asFloat()});

					log("Scores retrieved successfully", "Online");
				}
				else log("Error getting scores", "Online");

				log("Finished checking scores", "Online");
				scoresBeingChecked[mLevelValidator] = false;
			}).launch();

			return *result;
		}

		void setUpdatesChecked(bool mUpdatesChecked) { updatesChecked = mUpdatesChecked; }
		bool getUpdatesChecked() { return updatesChecked; }
		void setServerVersion(float mServerVersion) { serverVersion = mServerVersion; }
		float getServerVersion() { return serverVersion; }

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
			for(unsigned int i{0}; i < mString.size(); ++i) if(i % 9 == 0) result += mString[i];
			return result;
		}
	}
}


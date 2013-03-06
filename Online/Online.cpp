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
	bool updatesChecked{false};
	float serverVersion{-1};

	void checkUpdates()
	{
		Http http;
		http.setHost("http://vittorioromeo.info");
		Http::Request request("Misc/Linked/OHServer/OHInfo.json");
		Http::Response response{http.sendRequest(request)};
		Http::Response::Status status{response.getStatus()};
		if (status == Http::Response::Ok)
		{
			Json::Value root; Json::Reader reader; reader.parse(response.getBody(), root);

			string message(getJsonValueOrDefault<string>(root, "message", ""));
			log("Message: " + message, "Online");

			setServerVersion(getJsonValueOrDefault<float>(root, "latest_version", -1));
			log("Server latest version: " + toStr(getServerVersion()), "Online");

			if(getServerVersion() == getVersion()) log("No updates available", "Online");
			else if(getServerVersion() < getVersion()) log("Your version is newer than the server's (beta)", "Online");
			else if(getServerVersion() > getVersion()) log("Update available (" + toStr(getServerVersion()) + ")", "Online");
		}
		else
		{
			setServerVersion(-1);
			log("Error contacting the server", "Online");
		}

		setUpdatesChecked(true);
	}

	void setUpdatesChecked(bool mUpdatesChecked) { updatesChecked = mUpdatesChecked; }
	bool getUpdatesChecked() { return updatesChecked; }
	void setServerVersion(float mServerVersion) { serverVersion = mServerVersion; }
	float getServerVersion() { return serverVersion; }
}


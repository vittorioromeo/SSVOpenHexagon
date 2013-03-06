// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <string>
#include <random>
#include <SSVStart.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include "Global/Assets.h"
#include "Global/Config.h"
#include "HexagonGame.h"
#include "MenuGame.h"
#include "Utils/Utils.h"

using namespace std;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sf;
using namespace hg;

void checkUpdates()
{
	Http http;
	http.setHost("http://vittorioromeo.info");
	Http::Request request("Misc/Linked/OHServer/OHVersion.txt");
	Http::Response response{http.sendRequest(request)};
	Http::Response::Status status{response.getStatus()};
	if (status == Http::Response::Ok)
	{
		float serverVersion{lexicalCast<float>(response.getBody())};
		setServerVersion(serverVersion);

		if(serverVersion == getVersion()) log("No updates available", "Updates");
		else if(serverVersion < getVersion()) log("Your version is newer than the server's (beta)", "Updates");
		else if(serverVersion > getVersion()) log("Update available (" + toStr(serverVersion) + ")", "Updates");
	}
	else
	{
		cout << "Error " << status << endl;
	}

	setUpdatesChecked(true);
}

int main(int argc, char* argv[])
{
	sf::Thread thread(&checkUpdates); thread.launch();

	srand(unsigned(time(NULL)));

	vector<string> overrideIds; for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});
	loadConfig(overrideIds); initAssetManager(); loadAssets();

	string title{"Open Hexagon " + toStr<float>(getVersion()) + " - vee software"};
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getFullscreen()};
	window.setStaticFrameTime(getStaticFrameTime());
	window.setStaticFrameTimeValue(getStaticFrameTimeValue());
	window.setVsync(getVsync());
	window.setMouseCursorVisible(false);

	HexagonGame hg{window}; MenuGame mg{hg, window}; hg.mgPtr = &mg;

	window.setGameState(mg.getGame()); mg.init();
	window.run();

	saveConfig(); saveCurrentProfile(); saveLogToFile("log.txt");
	return 0;
}

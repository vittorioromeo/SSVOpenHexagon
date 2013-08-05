// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Server.h"
#include "SSVOpenHexagon/Online/OHServer.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Core/MenuGame.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"

using namespace std;
using namespace ssvs;
using namespace ssvu;
using namespace ssvu::FileSystem;
using namespace hg;

void createProfilesFolder()
{
	if(exists("Profiles/")) return;

	lo << lt("::createProfilesFolder") << "Profiles folder does not exist, creating" << endl;
	createFolder("Profiles/");
}

int main(int argc, char* argv[])
{
	vector<string> overrideIds; for(int i{0}; i < argc; ++i) overrideIds.push_back(string{argv[i]});

	if(contains(overrideIds, "server"))
	{
		HGAssets levelOnlyAssets{true};
		Online::initalizeValidators(levelOnlyAssets);
		Online::OHServer ohServer;
		ohServer.start();
		return 0;
	}

	createProfilesFolder();

	Online::initializeClient();
	Online::tryConnectToServer();

	loadConfig(overrideIds);

	string title{"Open Hexagon " + toStr(getVersion()) + " - by vittorio romeo"};
	GameWindow window{title, createStaticTimer(window), getWidth(), getHeight(), getPixelMultiplier(), getFullscreen()};
	window.setVsync(getVsync());
	window.setFPSLimit(60); // TODO: make optional
	window.setMouseCursorVisible(false);

	HGAssets assets;
	Online::initalizeValidators(assets);
	HexagonGame hg{assets, window};
	MenuGame mg{assets, hg, window};
	hg.mgPtr = &mg;

	assets.refreshVolumes();
	window.setGameState(mg.getGame()); mg.init();
	window.run();

	if(Online::getLoginStatus() != Online::LoginStat::Logged) Online::logout();

	saveConfig(); assets.pSaveCurrent(); saveLogToFile("log.txt");
	return 0;
}


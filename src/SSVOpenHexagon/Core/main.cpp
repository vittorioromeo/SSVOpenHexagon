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
	Path profilesPath{"Profiles/"};
	if(profilesPath.exists()) return;

	lo("::createProfilesFolder") << "Profiles folder does not exist, creating" << endl;
	createFolder(profilesPath);
}

int main(int argc, char* argv[])
{
	vector<string> overrideIds; for(int i{0}; i < argc; ++i) overrideIds.emplace_back(argv[i]);

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

	Config::loadConfig(overrideIds);

	GameWindow window;
	window.setTitle("Open Hexagon " + toStr(Config::getVersion()) + " - by vittorio romeo");
	Config::setTimerStatic(window, Config::getTimerStatic());
	window.setSize(Config::getWidth(), Config::getHeight());
	window.setPixelMult(Config::getPixelMultiplier());
	window.setFullscreen(Config::getFullscreen());
	window.setVsync(Config::getVsync());
	window.setFPSLimited(Config::getLimitFPS());
	window.setMaxFPS(Config::getMaxFPS());
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

	Config::saveConfig(); assets.pSaveCurrent(); saveLogToFile("log.txt");
	return 0;
}


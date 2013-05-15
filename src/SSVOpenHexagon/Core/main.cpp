// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <string>
#include <SSVUtils/SSVUtils.h>
#include <SSVStart/SSVStart.h>
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Core/MenuGame.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Compatibility/Compatibility.h"

using namespace std;
using namespace ssvs;
using namespace ssvu;
using namespace ssvu::FileSystem;
using namespace hg;

void createProfilesFolder()
{
	if(exists("Profiles/")) return;

	log("Profiles folder does not exist, creating", "CreateProfilesFolder");
	createFolder("Profiles/");
}

int main(int argc, char* argv[])
{
	vector<string> overrideIds; for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});

	createProfilesFolder();

	Online::startCheckUpdates();
	setRandomSeed();
	loadConfig(overrideIds); initAssetManager(); loadAssets();

	string title{"Open Hexagon " + toStr(getVersion()) + " - by vittorio romeo"};
	GameWindow window{title, createDynamicTimer(window), getWidth(), getHeight(), getPixelMultiplier(), getFullscreen()};
	window.setVsync(getVsync());
	window.setMouseCursorVisible(false);

	HexagonGame hg{window}; MenuGame mg{hg, window}; hg.mgPtr = &mg;

	window.setGameState(mg.getGame()); mg.init();
	window.run();

	Online::terminateAll();

	saveConfig(); saveCurrentProfile(); saveLogToFile("log.txt");
	return 0;
}

// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <SSVStart.h>
#include <SFML/System.hpp>
#include "Online/Online.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "HexagonGame.h"
#include "MenuGame.h"
#include "Online/Definitions.h"
#include "Compatibility/Compatibility.h"

using namespace std;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sf;
using namespace hg;

int main(int argc, char* argv[])
{
	vector<string> overrideIds; for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});

	Online::startCheckUpdates();
	srand(unsigned(time(NULL)));
	loadConfig(overrideIds); initAssetManager(); loadAssets();

	if(overrideIds[1] == "convert182to183")
	{
		Compatibility::convert182to183Hashes("_CONVERT/from.json", "_CONVERT/to.json");
		saveLogToFile("log.txt");
		return 0;
	}

	string title{"Open Hexagon " + toStr<float>(getVersion()) + " - vee software"};
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getFullscreen()};
	window.setStaticFrameTime(getStaticFrameTime());
	window.setStaticFrameTimeValue(getStaticFrameTimeValue());
	window.setVsync(getVsync());
	window.setMouseCursorVisible(false);

	HexagonGame hg{window}; MenuGame mg{hg, window}; hg.mgPtr = &mg;

	window.setGameState(mg.getGame()); mg.init();
	window.run();

	Online::terminateAll();

	saveConfig(); saveCurrentProfile(); saveLogToFile("log.txt");
	return 0;
}

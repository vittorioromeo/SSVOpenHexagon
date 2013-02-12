// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include <SSVStart.h>
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"

using namespace std;
using namespace sses;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sf;
using namespace hg;

int main(int argc, char* argv[])
{
	srand(unsigned(time(NULL)));

	vector<string> overrideIds;
	for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});

	loadConfig(overrideIds);
	initAssetManager();
	loadAssets();

	string title{"Open Hexagon " + toStr<float>(getVersion()) + " - vee software"};
	
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getFullscreen()};
	window.setStaticFrameTime(getStaticFrameTime());
	window.setStaticFrameTimeValue(getStaticFrameTimeValue());
	window.setVsync(getVsync());
	window.setMouseCursorVisible(false);

	HexagonGame hg{window};
	MenuGame mg{hg, window};

	hg.mgPtr = &mg;

	window.setGame(&mg.getGame());
	mg.init();

	window.run();

	saveConfig();
	saveCurrentProfile();	
	saveLogToFile("log.txt");

	return 0;
}

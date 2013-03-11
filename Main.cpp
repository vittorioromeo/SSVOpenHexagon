// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <string>
#include <random>
#include <SSVStart.h>
#include <SFML/System.hpp>
#include "Online/Online.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "HexagonGame.h"
#include "MenuGame.h"

using namespace std;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sf;
using namespace hg;

void convertHashes()
{
	for(auto& levelData : getAllLevelData())
	{
		for(float difficultyMult : levelData.getDifficultyMultipliers())
		{
			log("");
			log("");

			log("computing old validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
			string oldValidator{Online::get181Validator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
			log(oldValidator);
			log("");

			log("computing new validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
			string newValidator{Online::getValidator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
			log(newValidator);
			log("");
		}
	}
}

int main(int argc, char* argv[])
{	
	Online::startCheckUpdates();

	srand(unsigned(time(NULL)));

	vector<string> overrideIds; for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});
	loadConfig(overrideIds); initAssetManager(); loadAssets();

	string title{"Open Hexagon " + toStr<float>(getVersion()) + " - vee software"};
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getFullscreen()};
	window.setStaticFrameTime(getStaticFrameTime());
	window.setStaticFrameTimeValue(getStaticFrameTimeValue());
	window.setVsync(getVsync());
	window.setMouseCursorVisible(false);

	if(true) convertHashes();
	else
	{
		HexagonGame hg{window}; MenuGame mg{hg, window}; hg.mgPtr = &mg;

		window.setGameState(mg.getGame()); mg.init();
		window.run();
	}

	Online::terminateAll();

	saveConfig(); saveCurrentProfile(); saveLogToFile("log.txt");
	return 0;
}

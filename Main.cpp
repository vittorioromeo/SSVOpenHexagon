#include <vector>
#include <string>
#include <iostream>
#include <random>
#include "SSVEntitySystem.h"
#include "SSVStart.h"
#include "CPlayer.h"
#include "HexagonGame.h"
#include "MenuGame.h"
#include <memory>
#include "Config.h"
#include <json/json.h>
#include <json/reader.h>
#include "Utils.h"
#include "Assets.h"

using namespace std;
using namespace sses;
using namespace ssvs;
using namespace sf;
using namespace hg;

int main(int argc, char* argv[])
{
	vector<string> overrideIds;

	for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});

	loadConfigs(); // config overrides	
	loadConfig(overrideIds);
	loadAssets();

	srand(unsigned(time(NULL)));

	string title{"Open Hexagon " + getVersion() + " - vee software"};
	bool fullscreen{getFullscreen()};
	
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getLimitFps(), fullscreen};
	window.isFrameTimeStatic = getStaticFrameTime();
	window.staticFrameTime = getStaticFrameTimeValue();
	window.renderWindow.setVerticalSyncEnabled(getVsync());

	MenuGame mg{window};
	HexagonGame hg{window};

	mg.hgPtr = &hg;
	hg.mgPtr = &mg;

	window.setGame(&mg.getGame());
	mg.init();
	
	window.run();

	saveScores();

	return 0;
}

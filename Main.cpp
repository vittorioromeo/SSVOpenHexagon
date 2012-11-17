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

int main()
{
	loadConfig();
	loadAssets();

	srand(unsigned(time(NULL)));

	GameWindow window{(unsigned int)getWindowSizeX(), (unsigned int)getWindowSizeY(), getPixelMultiplier(), getLimitFps(), getFullscreen()};
	window.isFrameTimeStatic = getStaticFrameTime();
	window.staticFrameTime = getStaticFrameTimeValue();
	window.renderWindow.setVerticalSyncEnabled(getVsync());
	window.renderWindow.setTitle("Open Hexagon " + getVersion() + " - vee software");

	MenuGame mg{window};
	HexagonGame hg{window};

	mg.hexagonGamePtr = &hg;
	hg.menuGamePtr = &mg;

	window.setGame(&mg.getGame());
	mg.init();
	
	window.run();

	saveScores();

	return 0;
}

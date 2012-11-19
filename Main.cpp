#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <json/json.h>
#include <json/reader.h>
#include <SSVEntitySystem.h>
#include <SSVStart.h>
#include "Components/CPlayer.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "MenuGame.h"

using namespace std;
using namespace sses;
using namespace ssvs;
using namespace sf;
using namespace hg;

int main(int argc, char* argv[])
{
	vector<string> overrideIds;
	for(int i{0}; i < argc; i++) overrideIds.push_back(string{argv[i]});

	loadConfig(overrideIds);
	loadAssets();

	srand(unsigned(time(NULL)));

	string title{"Open Hexagon " + getVersion() + " - vee software"};
	
	GameWindow window{title, getWidth(), getHeight(), getPixelMultiplier(), getLimitFps(), getFullscreen()};
	window.isFrameTimeStatic = getStaticFrameTime();
	window.staticFrameTime = getStaticFrameTimeValue();
	window.renderWindow.setVerticalSyncEnabled(getVsync());
	window.renderWindow.setMouseCursorVisible(false);

	MenuGame mg{window};
	HexagonGame hg{window};

	mg.hgPtr = &hg;
	hg.mgPtr = &mg;

	window.setGame(&mg.getGame());
	mg.init();
	
	window.run();

	saveCurrentProfile();
	return 0;
}

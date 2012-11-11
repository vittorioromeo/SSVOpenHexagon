#include <iostream>
#include <string>
#include <json/json.h>
#include <json/reader.h>
#include <fstream>

namespace hg
{

	void loadConfig();
	float getWindowSizeX();
	float getWindowSizeY();
	float getSizeX();
	float getSizeY();
	float getSpawnDistance();
	float getZoomFactor();
	int getPixelMultiplier();
	float getPlayerSpeed();
	float getPlayerFocusSpeed();
	float getPlayerSize();
}

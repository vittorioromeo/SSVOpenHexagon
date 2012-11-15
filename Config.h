#ifndef CONFIG_H_
#define CONFIG_H_

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
	bool getNoRotation();
	bool getNoBackground();
	bool getBlackAndWhite();
	bool getNoSound();
	bool getNoMusic();
	int getSoundVolume();
	int getMusicVolume();
	bool getStaticFrameTime();
	float getStaticFrameTimeValue();
	bool getLimitFps();
	bool getVsync();
}

#endif /* CONFIG_H_ */

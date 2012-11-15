#include <iostream>
#include <string>
#include <json/json.h>
#include <json/reader.h>
#include <fstream>
#include "Utils.h"
#include "Config.h"

using namespace std;

namespace hg
{
	int windowSizeX				{1024};
	int windowSizeY				{768};
	float sizeX					{windowSizeX * 1.3f};
	float sizeY					{windowSizeX * 1.3f};
	float spawnDistance			{900};
	float zoomFactor			{1};
	int pixelMultiplier			{1};
	float playerSpeed			{8.3f};
	float playerFocusSpeed		{4.15f};
	float playerSize			{7};
	bool noRotation				{false};
	bool noBackground			{false};
	bool blackAndWhite			{false};
	bool noSound				{false};
	bool noMusic				{false};
	int soundVolume				{100};
	int musicVolume				{100};
	bool staticFrameTime		{false};
	float staticFrameTimeValue	{false};
	bool limitFps				{false};
	bool vsync					{false};

	void loadConfig()
	{
		log("loading config");

		Json::Value root;
		Json::Reader reader;
		ifstream test("config.json", std::ifstream::binary);

		bool parsingSuccessful = reader.parse( test, root, false );
		if (!parsingSuccessful) cout << reader.getFormatedErrorMessages() << endl;

		windowSizeX = 			root["window_size_x"].asInt();
		windowSizeY = 			root["window_size_y"].asInt();
		zoomFactor = 			root["zoom_factor"].asFloat();
		pixelMultiplier = 		root["pixel_multiplier"].asInt();
		playerSpeed = 			root["player_speed"].asFloat();
		playerFocusSpeed = 		root["player_focus_speed"].asFloat();
		playerSize = 			root["player_size"].asFloat();
		noRotation = 			root["no_rotation"].asBool();
		noBackground = 			root["no_background"].asBool();
		blackAndWhite = 		root["black_and_white"].asBool();
		noSound = 				root["no_sound"].asBool();
		noMusic = 				root["no_music"].asBool();
		soundVolume = 			root["sound_volume"].asInt();
		musicVolume = 			root["music_volume"].asInt();
		staticFrameTime = 		root["static_frametime"].asBool();
		staticFrameTimeValue = 	root["static_frametime_value"].asFloat();
		limitFps = 				root["limit_fps"].asBool();
		vsync = 				root["vsync"].asBool();

		sizeX = max(windowSizeX, windowSizeY) * 1.3f;
		sizeY = max(windowSizeX, windowSizeY)  * 1.3f;
		spawnDistance = max(windowSizeX, windowSizeY) * pixelMultiplier;
	}

	float getWindowSizeX() 			{ return windowSizeX; }
	float getWindowSizeY() 			{ return windowSizeY; }
	float getSizeX() 				{ return sizeX; }
	float getSizeY() 				{ return sizeY; }
	float getSpawnDistance() 		{ return spawnDistance; }
	float getZoomFactor() 			{ return zoomFactor; }
	int getPixelMultiplier() 		{ return pixelMultiplier; }
	float getPlayerSpeed() 			{ return playerSpeed; }
	float getPlayerFocusSpeed() 	{ return playerFocusSpeed; }
	float getPlayerSize() 			{ return playerSize; }
	bool getNoRotation() 			{ return noRotation; }
	bool getNoBackground() 			{ return noBackground; }
	bool getBlackAndWhite() 		{ return blackAndWhite; }
	bool getNoSound()				{ return noSound; }
	bool getNoMusic()				{ return noMusic; }
	int getSoundVolume()  			{ return soundVolume; }
	int getMusicVolume() 			{ return musicVolume; }	
	bool getStaticFrameTime()		{ return staticFrameTime; }
	float getStaticFrameTimeValue()	{ return staticFrameTimeValue; }
	bool getLimitFps()				{ return limitFps; }
	bool getVsync()					{ return vsync; }
}

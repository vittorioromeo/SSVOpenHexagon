#include <iostream>
#include <string>
#include <json/json.h>
#include <json/reader.h>
#include <fstream>
#include "Utils.h"
#include "Config.h"
#include "Assets.h"

using namespace std;

namespace hg
{	
	float sizeX						{1500};
	float sizeY						{1500};
	float spawnDistance				{950};
	float zoomFactor				{1};
	int pixelMultiplier				{1};
	float playerSpeed				{8.3f};
	float playerFocusSpeed			{4.15f};
	float playerSize				{7};
	bool noRotation					{false};
	bool noBackground				{false};
	bool blackAndWhite				{false};
	bool noSound					{false};
	bool noMusic					{false};
	int soundVolume					{100};
	int musicVolume					{100};
	bool staticFrameTime			{false};
	float staticFrameTimeValue		{false};
	bool limitFps					{false};
	bool vsync						{false};
	bool autoZoomFactor				{true};
	bool fullscreen					{true};
	bool windowedAutoResolution		{false};
	bool fullscreenAutoResolution 	{true};
	unsigned int fullscreenWidth 	{1024};
	unsigned int fullscreenHeight 	{768};
	unsigned int windowedWidth 		{1024};
	unsigned int windowedHeight		{768};

	void loadConfig(vector<string> mOverridesIds)
	{
		log("loading config");

		Json::Value root;
		Json::Reader reader;
		ifstream test("config.json", std::ifstream::binary);

		bool parsingSuccessful = reader.parse( test, root, false );
		if (!parsingSuccessful) cout << reader.getFormatedErrorMessages() << endl;

		log("applying config overrides");

		for(string overrideId : mOverridesIds)
		{
			Json::Value overrideRoot{getConfigRoot(overrideId)};

			for(Json::ValueIterator itr{overrideRoot.begin()}; itr != overrideRoot.end(); itr++)
				root[itr.key().asString()] = *itr;
		}

		zoomFactor = 				root["zoom_factor"].asFloat();
		pixelMultiplier = 			root["pixel_multiplier"].asInt();
		playerSpeed = 				root["player_speed"].asFloat();
		playerFocusSpeed = 			root["player_focus_speed"].asFloat();
		playerSize = 				root["player_size"].asFloat();
		noRotation = 				root["no_rotation"].asBool();
		noBackground = 				root["no_background"].asBool();
		blackAndWhite = 			root["black_and_white"].asBool();
		noSound = 					root["no_sound"].asBool();
		noMusic = 					root["no_music"].asBool();
		soundVolume = 				root["sound_volume"].asInt();
		musicVolume = 				root["music_volume"].asInt();
		staticFrameTime = 			root["static_frametime"].asBool();
		staticFrameTimeValue = 		root["static_frametime_value"].asFloat();
		limitFps = 					root["limit_fps"].asBool();
		vsync = 					root["vsync"].asBool();
		autoZoomFactor = 			root["auto_zoom_factor"].asBool();
		fullscreen = 				root["fullscreen"].asBool();
		windowedAutoResolution =	root["windowed_auto_resolution"].asBool();
		windowedWidth = 			root["windowed_width"].asInt();
		windowedHeight = 			root["windowed_height"].asInt();
		fullscreenAutoResolution =	root["fullscreen_auto_resolution"].asBool();
		fullscreenWidth = 			root["fullscreen_width"].asInt();
		fullscreenHeight = 			root["fullscreen_height"].asInt();

		if(windowedAutoResolution)
		{
			windowedWidth = VideoMode::getDesktopMode().width;
			windowedHeight = VideoMode::getDesktopMode().height;
		}
		if(fullscreenAutoResolution)
		{
			fullscreenWidth = VideoMode::getDesktopMode().width;
			fullscreenHeight = VideoMode::getDesktopMode().height;
		}

		sizeX = max(getWidth(), getHeight()) * 1.3f;
		sizeY = max(getWidth(), getHeight()) * 1.3f;

		if(autoZoomFactor)
		{
			float zoomFactorX(1024.0f / (float)getWidth());
			float zoomFactorY(768.0f / (float)getHeight());
			zoomFactor = max(zoomFactorX, zoomFactorY);
		}
	}

	float getSizeX() 					{ return sizeX; }
	float getSizeY() 					{ return sizeY; }
	float getSpawnDistance() 			{ return spawnDistance; }
	float getZoomFactor() 				{ return zoomFactor; }
	int getPixelMultiplier() 			{ return pixelMultiplier; }
	float getPlayerSpeed() 				{ return playerSpeed; }
	float getPlayerFocusSpeed() 		{ return playerFocusSpeed; }
	float getPlayerSize() 				{ return playerSize; }
	bool getNoRotation() 				{ return noRotation; }
	bool getNoBackground() 				{ return noBackground; }
	bool getBlackAndWhite() 			{ return blackAndWhite; }
	bool getNoSound()					{ return noSound; }
	bool getNoMusic()					{ return noMusic; }
	int getSoundVolume()  				{ return soundVolume; }
	int getMusicVolume() 				{ return musicVolume; }
	bool getStaticFrameTime()			{ return staticFrameTime; }
	float getStaticFrameTimeValue()		{ return staticFrameTimeValue; }
	bool getLimitFps()					{ return limitFps; }
	bool getVsync()						{ return vsync; }
	bool getAutoZoomFactor()			{ return autoZoomFactor; }
	bool getFullscreen()				{ return fullscreen; }	
	string getVersion() 				{ return "v1.1"; }
	bool getWindowedAutoResolution()	{ return windowedAutoResolution; }
	bool getFullscreenAutoResolution() 	{ return fullscreenAutoResolution; }
	unsigned int getFullscreenWidth()	{ return fullscreenWidth; }
	unsigned int getFullscreenHeight() 	{ return fullscreenHeight; }
	unsigned int getWindowedWidth()		{ return windowedWidth; }
	unsigned int getWindowedHeight()	{ return windowedHeight; }
	unsigned int getWidth() 			{ if(fullscreen) return fullscreenWidth; else return windowedWidth; }
	unsigned int getHeight() 			{ if(fullscreen) return fullscreenHeight; else return windowedHeight; }
	void setFullscreen(GameWindow& mWindow, bool mFullscreen)
	{
		fullscreen = mFullscreen;

		sizeX = max(getWidth(), getHeight()) * 1.3f;
		sizeY = max(getWidth(), getHeight()) * 1.3f;

		if(autoZoomFactor)
		{
			float zoomFactorX(1024.0f / (float)getWidth());
			float zoomFactorY(768.0f / (float)getHeight());
			zoomFactor = max(zoomFactorX, zoomFactorY);
		}

		mWindow.setSize(getWidth(), getHeight());
		mWindow.setFullscreen(fullscreen);
	}
}

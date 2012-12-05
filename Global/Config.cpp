/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include "Global/Config.h"
#include "Global/Assets.h"
#include "Utils/Utils.h"

using namespace std;

namespace hg
{
	Json::Value root{getJsonFileRoot("config.json")};
	map<string, Json::Value> configOverridesRootMap;

	float sizeX						{1500};
	float sizeY						{1500};
	float spawnDistance				{1600};
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

	void loadConfig(vector<string> mOverridesIds)
	{
		log("loading config");

		for(auto filePath : getAllFilePaths("ConfigOverrides/", ".json"))
		{
			string fileName{getFileNameFromFilePath(filePath, "ConfigOverrides/", ".json")};
			configOverridesRootMap.insert(make_pair(fileName, getJsonFileRoot(filePath)));
		}

		for(string overrideId : mOverridesIds)
		{
			Json::Value overrideRoot{configOverridesRootMap.find(overrideId)->second};

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

		if(getWindowedAutoResolution())
		{
			root["windowed_width"] = VideoMode::getDesktopMode().width;
			root["windowed_height"] = VideoMode::getDesktopMode().height;
		}
		if(getFullscreenAutoResolution())
		{
			root["fullscreen_width"] = VideoMode::getDesktopMode().width;
			root["fullscreen_height"] = VideoMode::getDesktopMode().height;
		}

		recalculateSizes();
	}

	void recalculateSizes()
	{
		sizeX = max(getWidth(), getHeight()) * 1.3f;
		sizeY = max(getWidth(), getHeight()) * 1.3f;

		if(getAutoZoomFactor())
		{
			float zoomFactorX(1024.0f / (float)getWidth());
			float zoomFactorY(768.0f / (float)getHeight());
			zoomFactor = max(zoomFactorX, zoomFactorY);
		}
	}
	void setFullscreen(GameWindow& mWindow, bool mFullscreen)
	{
		root["fullscreen"] = mFullscreen;

		recalculateSizes();

		mWindow.setSize(getWidth(), getHeight());
		mWindow.setFullscreen(getFullscreen());
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
	bool getAutoZoomFactor()			{ return root["auto_zoom_factor"].asBool(); }
	bool getFullscreen()				{ return root["fullscreen"].asBool(); }
	float getVersion() 					{ return 1.5f; }
	bool getWindowedAutoResolution()	{ return root["windowed_auto_resolution"].asBool(); }
	bool getFullscreenAutoResolution() 	{ return root["fullscreen_auto_resolution"].asBool(); }
	unsigned int getFullscreenWidth()	{ return root["fullscreen_width"].asInt(); }
	unsigned int getFullscreenHeight() 	{ return root["fullscreen_height"].asInt(); }
	unsigned int getWindowedWidth()		{ return root["windowed_width"].asInt(); }
	unsigned int getWindowedHeight()	{ return root["windowed_height"].asInt(); }
	unsigned int getWidth() 			{ if(getFullscreen()) return getFullscreenWidth(); else return getWindowedWidth(); }
	unsigned int getHeight() 			{ if(getFullscreen()) return getFullscreenHeight(); else return getWindowedHeight(); }
	bool getShowMessages()				{ return root["show_messages"].asBool(); }
	bool getChangeStyles()				{ return root["change_styles"].asBool(); }
	bool getChangeMusic()				{ return root["change_music"].asBool(); }
	bool getDebug()						{ return root["debug"].asBool(); }
	bool getPulse()						{ return root["pulse_enabled"].asBool(); }
	bool getBeatPulse()					{ return root["beatpulse_enabled"].asBool(); }
}

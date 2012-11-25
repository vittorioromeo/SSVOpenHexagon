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
	map<string, Json::Value> configOverridesRootMap;

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
	bool no3DEffects				{false};
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
	bool showMessages				{true};
	bool changeStyles				{true};
	bool changeMusic				{true};

	void loadConfig(vector<string> mOverridesIds)
	{
		log("loading config");

		Json::Value root{getJsonFileRoot("config.json")};

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
		no3DEffects =				root["no_3d_effects"].asBool();
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
		showMessages =				root["show_messages"].asBool();
		changeStyles =				root["change_styles"].asBool();
		changeMusic = 				root["change_music"].asBool();

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

		recalculateSizes();
	}

	void recalculateSizes()
	{
		sizeX = max(getWidth(), getHeight()) * 1.3f;
		sizeY = max(getWidth(), getHeight()) * 1.3f;

		if(autoZoomFactor)
		{
			float zoomFactorX(1024.0f / (float)getWidth());
			float zoomFactorY(768.0f / (float)getHeight());
			zoomFactor = max(zoomFactorX, zoomFactorY);
		}
	}
	void setFullscreen(GameWindow& mWindow, bool mFullscreen)
	{
		fullscreen = mFullscreen;

		recalculateSizes();

		mWindow.setSize(getWidth(), getHeight());
		mWindow.setFullscreen(fullscreen);
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
	bool getNo3DEffects()				{ return no3DEffects; }
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
	string getVersion() 				{ return "v1.3"; }
	bool getWindowedAutoResolution()	{ return windowedAutoResolution; }
	bool getFullscreenAutoResolution() 	{ return fullscreenAutoResolution; }
	unsigned int getFullscreenWidth()	{ return fullscreenWidth; }
	unsigned int getFullscreenHeight() 	{ return fullscreenHeight; }
	unsigned int getWindowedWidth()		{ return windowedWidth; }
	unsigned int getWindowedHeight()	{ return windowedHeight; }
	unsigned int getWidth() 			{ if(fullscreen) return fullscreenWidth; else return windowedWidth; }
	unsigned int getHeight() 			{ if(fullscreen) return fullscreenHeight; else return windowedHeight; }
	bool getShowMessages()				{ return showMessages; }
	bool getChangeStyles()				{ return changeStyles; }
	bool getChangeMusic()				{ return changeMusic; }
}

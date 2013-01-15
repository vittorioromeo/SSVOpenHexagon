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
using namespace ssvs;
using namespace ssvs::FileSystem;

namespace hg
{
	Json::Value root{getJsonFileRoot("config.json")};
	map<string, Json::Value> configOverridesRootMap;

	float sizeX						{1500};
	float sizeY						{1500};
	constexpr float spawnDistance	{1600};
	
	void loadConfig(vector<string> mOverridesIds)
	{
		log("loading config", "CONFIG");

		for(auto filePath : getFilesByExtension("ConfigOverrides/", ".json"))
		{
			string fileName{getNameFromPath(filePath, "ConfigOverrides/", ".json")};
			configOverridesRootMap.insert(make_pair(fileName, getJsonFileRoot(filePath)));
		}

		for(string overrideId : mOverridesIds)
		{
			Json::Value overrideRoot{configOverridesRootMap.find(overrideId)->second};

			for(Json::ValueIterator itr{overrideRoot.begin()}; itr != overrideRoot.end(); itr++)
				root[itr.key().asString()] = *itr;
		}

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
	void saveConfig()
	{
		// Seems like JSONcpp doesn't have a way to change a single value in an existing file - I'll just
		// replace the pulse effect option manually for now

		fstream f; f.open("config.json"); stringstream buffer; buffer << f.rdbuf(); f.close();

		string original{buffer.str()};
		if(getPulse()) original = replace(original, R"("pulse_enabled": false,)", R"("pulse_enabled": true,)");
		else original = replace(original, R"("pulse_enabled": true,)", R"("pulse_enabled": false,)");

		f.open("config.json", fstream::out | fstream::trunc); f << original; f.flush(); f.close();
	}

	void recalculateSizes()
	{
		sizeX = max(getWidth(), getHeight()) * 1.3f;
		sizeY = max(getWidth(), getHeight()) * 1.3f;

		if(getAutoZoomFactor())
		{
			float zoomFactorX(1024.0f / (float)getWidth());
			float zoomFactorY(768.0f / (float)getHeight());
			root["zoom_factor"] = max(zoomFactorX, zoomFactorY);
		}
	}
	void setFullscreen(GameWindow& mWindow, bool mFullscreen)
	{
		root["fullscreen"] = mFullscreen;

		recalculateSizes();

		mWindow.setSize(getWidth(), getHeight());
		mWindow.setFullscreen(getFullscreen());
	}

	void setPulse(bool mPulse) 			{ root["pulse_enabled"] = mPulse; }

	float getSizeX() 					{ return sizeX; }
	float getSizeY() 					{ return sizeY; }
	float getSpawnDistance() 			{ return spawnDistance; }
	float getZoomFactor() 				{ return root["zoom_factor"].asFloat(); }
	int getPixelMultiplier() 			{ return root["pixel_multiplier"].asInt(); }
	float getPlayerSpeed() 				{ return root["player_speed"].asFloat(); }
	float getPlayerFocusSpeed() 		{ return root["player_focus_speed"].asFloat(); }
	float getPlayerSize() 				{ return root["player_size"].asFloat(); }
	bool getNoRotation() 				{ return root["no_rotation"].asBool(); }
	bool getNoBackground() 				{ return root["no_background"].asBool(); }
	bool getBlackAndWhite() 			{ return root["black_and_white"].asBool(); }
	bool getNoSound()					{ return root["no_sound"].asBool(); }
	bool getNoMusic()					{ return root["no_music"].asBool(); }
	int getSoundVolume()  				{ return root["sound_volume"].asInt(); }
	int getMusicVolume() 				{ return root["music_volume"].asInt(); }
	bool getStaticFrameTime()			{ return root["static_frametime"].asBool(); }
	float getStaticFrameTimeValue()		{ return root["static_frametime_value"].asFloat(); }
	bool getLimitFps()					{ return root["limit_fps"].asBool(); }
	bool getVsync()						{ return root["vsync"].asBool(); }
	bool getAutoZoomFactor()			{ return root["auto_zoom_factor"].asBool(); }
	bool getFullscreen()				{ return root["fullscreen"].asBool(); }
	float getVersion() 					{ return 1.61f; }
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

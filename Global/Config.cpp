// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <string>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include "Global/Config.h"
#include "Global/Assets.h"
#include "Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
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

		if(getDebug()) return;
		fstream f; f.open("config.json"); stringstream buffer; buffer << f.rdbuf(); f.close();

		string original{buffer.str()};

		vector<string> elements{"no_rotation", "no_background", "black_and_white", "no_sound", "no_music", "pulse_enabled", "3D_enabled", "invincible"};
		vector<bool> predicates{getNoRotation(), getNoBackground(), getBlackAndWhite(), getNoSound(), getNoMusic(), getPulse(), get3D(), getInvincible()};

		for(unsigned int i{0}; i < elements.size(); ++i)
		{
			string element{"\"" + elements[i] + "\""};
			original = predicates[i] ? replace(original, element + ": false", element + ": true") : replace(original, element + ": true", element + ": false");
		}

		f.open("config.json", fstream::out | fstream::trunc); f << original; f.flush(); f.close();
	}

	void recalculateSizes()
	{
		sizeX = max(getWidth(), getHeight()) * 1.3f;
		sizeY = max(getWidth(), getHeight()) * 1.3f;

		if(getAutoZoomFactor())
		{
			float zoomFactorX(1024.0f / static_cast<float>(getWidth()));
			float zoomFactorY(768.0f / static_cast<float>(getHeight()));
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

	void setNoRotation(bool mNoRotation)		{ root["no_rotation"] = mNoRotation; }
	void setNoBackground(bool mNoBackground)	{ root["no_background"] = mNoBackground; }
	void setBlackAndWhite(bool mBlackAndWhite)	{ root["black_and_white"] = mBlackAndWhite; }
	void setNoSound(bool mNoSound)				{ root["no_sound"] = mNoSound; }
	void setNoMusic(bool mNoMusic)				{ root["no_music"] = mNoMusic; }
	void setPulse(bool mPulse) 					{ root["pulse_enabled"] = mPulse; }
	void set3D(bool m3D)						{ root["3D_enabled"] = m3D; }
	void setInvincible(bool mInvincible)		{ root["invincible"] = mInvincible; }

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
	float getVersion() 					{ return 1.71f; }
	bool getWindowedAutoResolution()	{ return root["windowed_auto_resolution"].asBool(); }
	bool getFullscreenAutoResolution() 	{ return root["fullscreen_auto_resolution"].asBool(); }
	unsigned int getFullscreenWidth()	{ return root["fullscreen_width"].asInt(); }
	unsigned int getFullscreenHeight() 	{ return root["fullscreen_height"].asInt(); }
	unsigned int getWindowedWidth()		{ return root["windowed_width"].asInt(); }
	unsigned int getWindowedHeight()	{ return root["windowed_height"].asInt(); }
	unsigned int getWidth() 			{ return getFullscreen() ? getFullscreenWidth() : getWindowedWidth(); }
	unsigned int getHeight() 			{ return getFullscreen() ? getFullscreenHeight() : getWindowedHeight(); }
	bool getShowMessages()				{ return root["show_messages"].asBool(); }
	bool getChangeStyles()				{ return root["change_styles"].asBool(); }
	bool getChangeMusic()				{ return root["change_music"].asBool(); }
	bool getDebug()						{ return root["debug"].asBool(); }
	bool getPulse()						{ return root["pulse_enabled"].asBool(); }
	bool getBeatPulse()					{ return root["beatpulse_enabled"].asBool(); }
	bool getInvincible()				{ return root["invincible"].asBool(); }
	bool get3D()						{ return root["3D_enabled"].asBool(); }
	float get3DMultiplier()				{ return root["3D_multiplier"].asFloat(); }
	unsigned int get3DMaxDepth()		{ return root["3D_max_depth"].asInt(); }
}

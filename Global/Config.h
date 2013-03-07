// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_CONFIG
#define HG_CONFIG

#include <string>
#include <SSVStart.h>

namespace hg
{
	void loadConfig(std::vector<std::string> mOverridesIds);
	void saveConfig();

	void recalculateSizes();
	void setFullscreen(ssvs::GameWindow& mWindow, bool mFullscreen);

	void setNoRotation(bool mNoRotation);		
	void setNoBackground(bool mNoBackground);
	void setBlackAndWhite(bool mBlackAndWhite);
	void setNoSound(bool mNoSound);
	void setNoMusic(bool mNoMusic);
	void setPulse(bool mPulse);
	void set3D(bool m3D);
	void setInvincible(bool mInvincible);
	void setAutoRestart(bool mAutoRestart);
	void set3DMultiplier(float m3DMultiplier);

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
	bool getAutoZoomFactor();
	bool getFullscreen();
	float getVersion();
	bool getWindowedAutoResolution();
	bool getFullscreenAutoResolution();
	unsigned int getFullscreenWidth();
	unsigned int getFullscreenHeight();
	unsigned int getWindowedWidth();
	unsigned int getWindowedHeight();
	unsigned int getWidth();
	unsigned int getHeight();
	bool getShowMessages();
	bool getChangeStyles();
	bool getChangeMusic();
	bool getDebug();
	bool getPulse();
	bool getBeatPulse();
	bool getInvincible();
	bool get3D();
	float get3DMultiplier();
	unsigned int get3DMaxDepth();
	bool getAutoRestart();
}

#endif

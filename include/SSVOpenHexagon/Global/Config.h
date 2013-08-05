// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_CONFIG
#define HG_CONFIG

#include "SSVOpenHexagon/Core/HGDependencies.h"

namespace hg
{
	namespace Config
	{
		void loadConfig(const std::vector<std::string>& mOverridesIds);
		void saveConfig();

		bool isEligibleForScore();

		void recalculateSizes();
		void setFullscreen(ssvs::GameWindow& mWindow, bool mFullscreen);

		void refreshWindowSize(unsigned int mWidth, unsigned int mHeight);
		void setCurrentResolution(ssvs::GameWindow& mWindow, unsigned int mWidth, unsigned int mHeight);
		void setCurrentResolutionAuto(ssvs::GameWindow& mWindow);
		void setVsync(ssvs::GameWindow& mWindow, bool mValue);

		void setOnline(bool mOnline);
		void setOfficial(bool mOfficial);
		void setNoRotation(bool mNoRotation);
		void setNoBackground(bool mNoBackground);
		void setBlackAndWhite(bool mBlackAndWhite);
		void setNoSound(bool mNoSound);
		void setNoMusic(bool mNoMusic);
		void setPulse(bool mPulse);
		void set3D(bool m3D);
		void setInvincible(bool mInvincible);
		void setAutoRestart(bool mAutoRestart);
		void setSoundVolume(int mVolume);
		void setMusicVolume(int mVolume);
		void setFlash(bool mFlash);
		void setMusicSpeedDMSync(bool mValue);
		void setLimitFPS(bool mValue);
		void setMaxFPS(unsigned int mValue);
		void setShowFPS(bool mValue);

		bool getOnline();
		bool getOfficial();
		std::string getUneligibilityReason();
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
		bool getLimitFPS();
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
		unsigned int get3DMaxDepth();
		float get3DMultiplier();
		bool getAutoRestart();
		bool getFlash();
		bool getShowTrackedVariables();
		bool getMusicSpeedDMSync();
		unsigned int getMaxFPS();
		bool getShowFPS();

		ssvs::Input::Trigger getTriggerRotateCCW();
		ssvs::Input::Trigger getTriggerRotateCW();
		ssvs::Input::Trigger getTriggerFocus();
		ssvs::Input::Trigger getTriggerExit();
		ssvs::Input::Trigger getTriggerForceRestart();
		ssvs::Input::Trigger getTriggerRestart();
		ssvs::Input::Trigger getTriggerScreenshot();
		ssvs::Input::Trigger getTriggerSwap();
	}
}

#endif

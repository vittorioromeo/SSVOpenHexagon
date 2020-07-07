// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <vector>
#include <string>

namespace ssvs
{
class GameWindow;
} // namespace ssvs

namespace ssvs::Input
{
class Trigger;
} // namespace ssvs::Input

namespace hg::Config
{

void loadConfig(const std::vector<std::string>& mOverridesIds);
void saveConfig();

[[nodiscard]] bool isEligibleForScore();

void recalculateSizes();
void setFullscreen(ssvs::GameWindow& mWindow, bool mFullscreen);

void refreshWindowSize(unsigned int mWidth, unsigned int mHeight);
void setCurrentResolution(
    ssvs::GameWindow& mWindow, unsigned int mWidth, unsigned int mHeight);
void setCurrentResolutionAuto(ssvs::GameWindow& mWindow);

void setVsync(ssvs::GameWindow& mWindow, bool mValue);
void setLimitFPS(ssvs::GameWindow& mWindow, bool mValue);
void setMaxFPS(ssvs::GameWindow& mWindow, unsigned int mValue);
void setTimerStatic(ssvs::GameWindow& mWindow, bool mValue);
void setAntialiasingLevel(ssvs::GameWindow& mWindow, unsigned int mValue);

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
void setSoundVolume(float mVolume);
void setMusicVolume(float mVolume);
void setFlash(bool mFlash);
void setMusicSpeedDMSync(bool mValue);
void setShowFPS(bool mValue);
void setServerLocal(bool mValue);
void setServerVerbose(bool mValue);
void setMouseVisible(bool mValue);
void setMusicSpeedMult(float mValue);
void setDrawTextOutlines(bool mX);
void setDarkenUnevenBackgroundChunk(bool mX);
void setRotateToStart(bool mX);
void setJoystickDeadzone(float mX);
void setTextPadding(float mX);
void setTextScaling(float mX);

[[nodiscard]] bool getOnline();
[[nodiscard]] bool getOfficial();
[[nodiscard]] std::string getUneligibilityReason();
[[nodiscard]] float getSizeX();
[[nodiscard]] float getSizeY();
[[nodiscard]] float getSpawnDistance();
[[nodiscard]] float getZoomFactor();
[[nodiscard]] int getPixelMultiplier();
[[nodiscard]] float getPlayerSpeed();
[[nodiscard]] float getPlayerFocusSpeed();
[[nodiscard]] float getPlayerSize();
[[nodiscard]] bool getNoRotation();
[[nodiscard]] bool getNoBackground();
[[nodiscard]] bool getBlackAndWhite();
[[nodiscard]] bool getNoSound();
[[nodiscard]] bool getNoMusic();
[[nodiscard]] float getSoundVolume();
[[nodiscard]] float getMusicVolume();
[[nodiscard]] bool getLimitFPS();
[[nodiscard]] bool getVsync();
[[nodiscard]] bool getAutoZoomFactor();
[[nodiscard]] bool getFullscreen();
[[nodiscard]] float getVersion();
[[nodiscard]] const char* getVersionString();
[[nodiscard]] bool getWindowedAutoResolution();
[[nodiscard]] bool getFullscreenAutoResolution();
[[nodiscard]] unsigned int getFullscreenWidth();
[[nodiscard]] unsigned int getFullscreenHeight();
[[nodiscard]] unsigned int getWindowedWidth();
[[nodiscard]] unsigned int getWindowedHeight();
[[nodiscard]] unsigned int getWidth();
[[nodiscard]] unsigned int getHeight();
[[nodiscard]] bool getShowMessages();
[[nodiscard]] bool getRotateToStart();

[[nodiscard]] bool getDebug();
[[nodiscard]] bool getPulse();
[[nodiscard]] bool getBeatPulse();
[[nodiscard]] bool getInvincible();
[[nodiscard]] bool get3D();
[[nodiscard]] unsigned int get3DMaxDepth();
[[nodiscard]] float get3DMultiplier();
[[nodiscard]] bool getAutoRestart();
[[nodiscard]] bool getFlash();
[[nodiscard]] bool getShowTrackedVariables();
[[nodiscard]] bool getMusicSpeedDMSync();
[[nodiscard]] unsigned int getMaxFPS();
[[nodiscard]] bool getShowFPS();
[[nodiscard]] bool getTimerStatic();
[[nodiscard]] unsigned int getAntialiasingLevel();
[[nodiscard]] bool getServerLocal();
[[nodiscard]] bool getServerVerbose();
[[nodiscard]] bool getMouseVisible();
[[nodiscard]] float getMusicSpeedMult();
[[nodiscard]] bool getDrawTextOutlines();
[[nodiscard]] bool getDarkenUnevenBackgroundChunk();
[[nodiscard]] float getJoystickDeadzone();
[[nodiscard]] float getTextPadding();
[[nodiscard]] float getTextScaling();

[[nodiscard]] ssvs::Input::Trigger getTriggerRotateCCW();
[[nodiscard]] ssvs::Input::Trigger getTriggerRotateCW();
[[nodiscard]] ssvs::Input::Trigger getTriggerFocus();
[[nodiscard]] ssvs::Input::Trigger getTriggerExit();
[[nodiscard]] ssvs::Input::Trigger getTriggerForceRestart();
[[nodiscard]] ssvs::Input::Trigger getTriggerRestart();
[[nodiscard]] ssvs::Input::Trigger getTriggerScreenshot();
[[nodiscard]] ssvs::Input::Trigger getTriggerSwap();
[[nodiscard]] ssvs::Input::Trigger getTriggerUp();
[[nodiscard]] ssvs::Input::Trigger getTriggerDown();

} // namespace hg::Config

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"

#include <vector>
#include <string>
#include <array>

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

inline constexpr GameVersion GAME_VERSION{2, 0, 3};

void loadConfig(const std::vector<std::string>& mOverridesIds);
void resetConfigToDefaults();
void resetBindsToDefaults();
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
void setDebug(bool mDebug);
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
void setTimescale(float mX);
void setShowKeyIcons(bool mX);
void setKeyIconsScale(float mX);
void setFirstTimePlaying(bool mX);
void setSaveLocalBestReplayToFile(bool mX);

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

[[nodiscard, gnu::const]] inline constexpr GameVersion getVersion()
{
    return GAME_VERSION;
}

[[nodiscard]] const std::string& getVersionString();

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
[[nodiscard]] float getTimescale();
[[nodiscard]] bool getShowKeyIcons();
[[nodiscard]] float getKeyIconsScale();
[[nodiscard]] bool getFirstTimePlaying();
[[nodiscard]] bool getSaveLocalBestReplayToFile();

// keyboard binds

enum Tid
{
    Unknown = -1,
    RotateCCW = 0,
    RotateCW,
    Focus,
    Select,
    Exit,
    ForceRestart,
    Restart,
    Replay,
    Screenshot,
    Swap,
    Up,
    Down,
    NextPack,
    PreviousPack,
    TriggersCount
};

void keyboardBindsSanityCheck();

[[nodiscard]] std::string getKeyboardBindNames(const int bindID);
[[nodiscard]] ssvs::Input::Trigger getTriggerRotateCCW();
[[nodiscard]] ssvs::Input::Trigger getTriggerRotateCW();
[[nodiscard]] ssvs::Input::Trigger getTriggerFocus();
[[nodiscard]] ssvs::Input::Trigger getTriggerSelect();
[[nodiscard]] ssvs::Input::Trigger getTriggerExit();
[[nodiscard]] ssvs::Input::Trigger getTriggerForceRestart();
[[nodiscard]] ssvs::Input::Trigger getTriggerRestart();
[[nodiscard]] ssvs::Input::Trigger getTriggerReplay();
[[nodiscard]] ssvs::Input::Trigger getTriggerScreenshot();
[[nodiscard]] ssvs::Input::Trigger getTriggerSwap();
[[nodiscard]] ssvs::Input::Trigger getTriggerUp();
[[nodiscard]] ssvs::Input::Trigger getTriggerDown();
[[nodiscard]] ssvs::Input::Trigger getTriggerNextPack();
[[nodiscard]] ssvs::Input::Trigger getTriggerPreviousPack();

using KeyboardTriggerGetter = ssvs::Input::Trigger (*)();
extern const std::array<KeyboardTriggerGetter, Tid::TriggersCount> keyboardTriggerGetters;

void addBindTriggerRotateCCW(const int key, const int btn, const int index);
void addBindTriggerRotateCW(const int key, const int btn, const int index);
void addBindTriggerFocus(const int key, const int btn, const int index);
void addBindTriggerSelect(const int key, const int btn, const int index);
void addBindTriggerExit(const int key, const int btn, const int index);
void addBindTriggerForceRestart(const int key, const int btn, const int index);
void addBindTriggerRestart(const int key, const int btn, const int index);
void addBindTriggerReplay(const int key, const int btn, const int index);
void addBindTriggerScreenshot(const int key, const int btn, const int index);
void addBindTriggerSwap(const int key, const int btn, const int index);
void addBindTriggerUp(const int key, const int btn, const int index);
void addBindTriggerDown(const int key, const int btn, const int index);
void addBindTriggerNextPack(const int key, const int btn, const int index);
void addBindTriggerPreviousPack(const int key, const int btn, const int index);

void setTriggerRotateCCW(ssvs::Input::Trigger& trig);
void setTriggerRotateCW(ssvs::Input::Trigger& trig);
void setTriggerFocus(ssvs::Input::Trigger& trig);
void setTriggerSelect(ssvs::Input::Trigger& trig);
void setTriggerExit(ssvs::Input::Trigger& trig);
void setTriggerForceRestart(ssvs::Input::Trigger& trig);
void setTriggerRestart(ssvs::Input::Trigger& trig);
void setTriggerReplay(ssvs::Input::Trigger& trig);
void setTriggerScreenshot(ssvs::Input::Trigger& trig);
void setTriggerSwap(ssvs::Input::Trigger& trig);
void setTriggerUp(ssvs::Input::Trigger& trig);
void setTriggerDown(ssvs::Input::Trigger& trig);
void setTriggerNextPack(ssvs::Input::Trigger& trig);
void setTriggerPreviousPack(ssvs::Input::Trigger& trig);

void clearBindTriggerRotateCCW(const int index);
void clearBindTriggerRotateCW(const int index);
void clearBindTriggerFocus(const int index);
void clearBindTriggerSelect(const int index);
void clearBindTriggerExit(const int index);
void clearBindTriggerForceRestart(const int index);
void clearBindTriggerRestart(const int index);
void clearBindTriggerReplay(const int index);
void clearBindTriggerScreenshot(const int index);
void clearBindTriggerSwap(const int index);
void clearBindTriggerUp(const int index);
void clearBindTriggerDown(const int index);
void clearBindTriggerNextPack(const int index);
void clearBindTriggerPreviousPack(const int index);

// joystick binds

void joystickBindsSanityCheck();

[[nodiscard]] const std::string getJoystickBindNames(const int bindID);
[[nodiscard]] unsigned int getJoystickSelect();
[[nodiscard]] unsigned int getJoystickExit();
[[nodiscard]] unsigned int getJoystickFocus();
[[nodiscard]] unsigned int getJoystickSwap();
[[nodiscard]] unsigned int getJoystickForceRestart();
[[nodiscard]] unsigned int getJoystickRestart();
[[nodiscard]] unsigned int getJoystickReplay();
[[nodiscard]] unsigned int getJoystickScreenshot();
[[nodiscard]] unsigned int getJoystickNextPack();
[[nodiscard]] unsigned int getJoystickPreviousPack();

using JoystickTriggerGetter = unsigned int (*)();
extern const std::array<JoystickTriggerGetter, hg::Joystick::Jid::JoystickBindsCount> joystickTriggerGetters;

[[nodiscard]] int reassignToJoystickSelect(const unsigned int button);
[[nodiscard]] int reassignToJoystickExit(const unsigned int button);
[[nodiscard]] int reassignToJoystickFocus(const unsigned int button);
[[nodiscard]] int reassignToJoystickSwap(const unsigned int button);
[[nodiscard]] int reassignToJoystickForceRestart(const unsigned int button);
[[nodiscard]] int reassignToJoystickRestart(const unsigned int button);
[[nodiscard]] int reassignToJoystickReplay(const unsigned int button);
[[nodiscard]] int reassignToJoystickScreenshot(const unsigned int button);
[[nodiscard]] int reassignToJoystickNextPack(const unsigned int button);
[[nodiscard]] int reassignToJoystickPreviousPack(const unsigned int button);

void setJoystickSelect(const unsigned int button);
void setJoystickExit(const unsigned int button);
void setJoystickFocus(const unsigned int button);
void setJoystickSwap(const unsigned int button);
void setJoystickForceRestart(const unsigned int button);
void setJoystickRestart(const unsigned int button);
void setJoystickReplay(const unsigned int button);
void setJoystickScreenshot(const unsigned int button);
void setJoystickNextPack(const unsigned int button);
void setJoystickPreviousPack(const unsigned int button);

} // namespace hg::Config

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Utils/Casts.hpp"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <vector>
#include <string>
#include <array>

namespace ssvs {
class GameWindow;
} // namespace ssvs

namespace ssvs::Input {
class Trigger;
} // namespace ssvs::Input

namespace hg::Config {

inline constexpr float TICKS_PER_SECOND = 240.f;
inline constexpr float TIME_STEP = 60.f / TICKS_PER_SECOND;
inline constexpr float TIME_SLICE = 60.f / TICKS_PER_SECOND;

void loadConfig(const std::vector<std::string>& mOverridesIds);
void reapplyResolution();
void resetConfigToDefaults();
void resetBindsToDefaults();
void saveConfig();

[[nodiscard]] bool isEligibleForScore();

void recalculateSizes();
void setFullscreen(ssvs::GameWindow& mWindow, bool mFullscreen);

void setCurrentResolution(unsigned int mWidth, unsigned int mHeight);
void setCurrentResolutionAuto(ssvs::GameWindow& mWindow);

void setVsync(ssvs::GameWindow& mWindow, bool mValue);
void setLimitFPS(ssvs::GameWindow& mWindow, bool mValue);
void setMaxFPS(ssvs::GameWindow& mWindow, unsigned int mValue);
void setAntialiasingLevel(ssvs::GameWindow& mWindow, unsigned int mValue);

void setOfficial(bool mOfficial);
void setDebug(bool mDebug);
void setNoPulse(bool mNoPulse);
void setNoRotation(bool mNoRotation);
void setNoBackground(bool mNoBackground);
void setBlackAndWhite(bool mBlackAndWhite);
void setNoSound(bool mNoSound);
void setNoMusic(bool mNoMusic);
void set3D(bool m3D);
void setShaders(bool mX);
void setInvincible(bool mInvincible);
void setAutoRestart(bool mAutoRestart);
void setSoundVolume(float mVolume);
void setMusicVolume(float mVolume);
void setFlash(bool mFlash);
void setMusicSpeedDMSync(bool mValue);
void setShowFPS(bool mValue);
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
void setShowLevelInfo(bool mX);
void setShowTimer(bool mX);
void setShowStatusText(bool mX);
void setServerIp(const std::string& mX);
void setServerPort(unsigned short mX);
void setServerControlPort(unsigned short mX);
void setServerLevelWhitelist(const std::vector<std::string>& levelValidators);
void setSaveLastLoginUsername(bool mX);
void setLastLoginUsername(const std::string& mX);
void setShowLoginAtStartup(bool mX);
void setCameraShakeMultiplier(float x);
void setAngleTiltIntensity(float x);
void setShowPlayerTrail(bool mX);
void setPlayerTrailAlpha(unsigned int x);
void setPlayerTrailScale(float x);
void setPlayerTrailDecay(float x);
void setPlayerTrailHasSwapColor(bool x);
void setShowSwapParticles(bool x);
void setPlaySwapReadySound(bool x);
void setShowSwapBlinkingEffect(bool x);
void setUseLuaFileCache(bool x);
void setDisableGameRendering(bool x);

[[nodiscard]] bool getOfficial();
[[nodiscard]] const std::string& getUneligibilityReason();
[[nodiscard]] float getSizeX();
[[nodiscard]] float getSizeY();
[[nodiscard]] float getSpawnDistance();
[[nodiscard]] float getZoomFactor();
[[nodiscard]] int getPixelMultiplier();
[[nodiscard]] float getPlayerSpeed();
[[nodiscard]] float getPlayerFocusSpeed();
[[nodiscard]] float getPlayerSize();
[[nodiscard]] bool getNoPulse();
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
[[nodiscard]] bool getDebug();
[[nodiscard]] bool getBeatPulse();
[[nodiscard]] bool getInvincible();
[[nodiscard]] bool get3D();
[[nodiscard]] bool getShaders();
[[nodiscard]] unsigned int get3DMaxDepth();
[[nodiscard]] float get3DMultiplier();
[[nodiscard]] bool getAutoRestart();
[[nodiscard]] bool getFlash();
[[nodiscard]] bool getShowTrackedVariables();
[[nodiscard]] bool getMusicSpeedDMSync();
[[nodiscard]] unsigned int getMaxFPS();
[[nodiscard]] bool getShowFPS();
[[nodiscard]] unsigned int getAntialiasingLevel();
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
[[nodiscard]] bool getShowLevelInfo();
[[nodiscard]] bool getShowTimer();
[[nodiscard]] bool getShowStatusText();
[[nodiscard]] const std::string& getServerIp();
[[nodiscard]] unsigned short getServerPort();
[[nodiscard]] unsigned short getServerControlPort();
[[nodiscard]] const std::vector<std::string>& getServerLevelWhitelist();
[[nodiscard]] bool getSaveLastLoginUsername();
[[nodiscard]] const std::string& getLastLoginUsername();
[[nodiscard]] bool getShowLoginAtStartup();
[[nodiscard]] float getCameraShakeMultiplier();
[[nodiscard]] float getAngleTiltIntensity();
[[nodiscard]] bool getShowPlayerTrail();
[[nodiscard]] unsigned int getPlayerTrailAlpha();
[[nodiscard]] float getPlayerTrailScale();
[[nodiscard]] float getPlayerTrailDecay();
[[nodiscard]] bool getPlayerTrailHasSwapColor();
[[nodiscard]] bool getShowSwapParticles();
[[nodiscard]] bool getPlaySwapReadySound();
[[nodiscard]] bool getShowSwapBlinkingEffect();
[[nodiscard]] bool getUseLuaFileCache();
[[nodiscard]] bool getDisableGameRendering();

// keyboard binds

enum class Tid : int
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
    LuaConsole,
    Pause,

    TriggersCount
};

void keyboardBindsSanityCheck();

[[nodiscard]] std::string getKeyboardBindNames(const Tid bindID);

using TriggerGetter = ssvs::Input::Trigger& (*)();
extern const std::array<TriggerGetter, toSizeT(Tid::TriggersCount)>
    triggerGetters;

void rebindTrigger(ssvs::Input::Trigger& trig, const sf::Keyboard::Key key,
    const sf::Mouse::Button btn, int index);

void clearTriggerBind(ssvs::Input::Trigger& trig, const int index);

// joystick binds

void joystickBindsSanityCheck();

[[nodiscard]] std::string getJoystickBindName(const Joystick::Jid bindID);

using JoystickTriggerGetter = unsigned int (*)();
extern const std::array<JoystickTriggerGetter,
    toSizeT(Joystick::Jid::JoystickBindsCount)>
    joystickTriggerGetters;

void loadAllJoystickBinds();

using JoystickTriggerSetter = void (*)(const unsigned int button);
extern const std::array<JoystickTriggerSetter,
    toSizeT(Joystick::Jid::JoystickBindsCount)>
    joystickTriggerSetters;

[[nodiscard]] ssvs::Input::Trigger& getTrigger(const Tid tid);

} // namespace hg::Config

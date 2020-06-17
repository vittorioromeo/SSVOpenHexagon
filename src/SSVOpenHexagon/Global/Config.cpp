// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <fstream>
#include <memory>
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Input;
using namespace ssvu::FileSystem;
using namespace ssvuj;
using namespace ssvu;

namespace hg
{
namespace Config
{
ssvuj::Obj root{getFromFile("config.json")};
LinkedValueManager lvm{root};

auto& online(lvm.create<bool>("online"));
auto& official(lvm.create<bool>("official"));
auto& noRotation(lvm.create<bool>("no_rotation"));
auto& noBackground(lvm.create<bool>("no_background"));
auto& noSound(lvm.create<bool>("no_sound"));
auto& noMusic(lvm.create<bool>("no_music"));
auto& blackAndWhite(lvm.create<bool>("black_and_white"));
auto& pulseEnabled(lvm.create<bool>("pulse_enabled"));
auto& _3DEnabled(lvm.create<bool>("3D_enabled"));
auto& _3DMultiplier(lvm.create<float>("3D_multiplier"));
auto& _3DMaxDepth(lvm.create<unsigned int>("3D_max_depth"));
auto& invincible(lvm.create<bool>("invincible"));
auto& autoRestart(lvm.create<bool>("auto_restart"));
auto& soundVolume(lvm.create<float>("sound_volume"));
auto& musicVolume(lvm.create<float>("music_volume"));
auto& flashEnabled(lvm.create<bool>("flash_enabled"));
auto& zoomFactor(lvm.create<float>("zoom_factor"));
auto& pixelMultiplier(lvm.create<int>("pixel_multiplier"));
auto& playerSpeed(lvm.create<float>("player_speed"));
auto& playerFocusSpeed(lvm.create<float>("player_focus_speed"));
auto& playerSize(lvm.create<float>("player_size"));
auto& limitFPS(lvm.create<bool>("limit_fps"));
auto& vsync(lvm.create<bool>("vsync"));
auto& autoZoomFactor(lvm.create<bool>("auto_zoom_factor"));
auto& fullscreen(lvm.create<bool>("fullscreen"));
auto& windowedAutoResolution(lvm.create<bool>("windowed_auto_resolution"));
auto& fullscreenAutoResolution(lvm.create<bool>("fullscreen_auto_resolution"));
auto& fullscreenWidth(lvm.create<unsigned int>("fullscreen_width"));
auto& fullscreenHeight(lvm.create<unsigned int>("fullscreen_height"));
auto& windowedWidth(lvm.create<unsigned int>("windowed_width"));
auto& windowedHeight(lvm.create<unsigned int>("windowed_height"));
auto& showMessages(lvm.create<bool>("show_messages"));
auto& debug(lvm.create<bool>("debug"));
auto& beatPulse(lvm.create<bool>("beatpulse_enabled"));
auto& showTrackedVariables(lvm.create<bool>("show_tracked_variables"));
auto& musicSpeedDMSync(lvm.create<bool>("music_speed_dm_sync"));
auto& maxFPS(lvm.create<unsigned int>("max_fps"));
auto& antialiasingLevel(lvm.create<unsigned int>("antialiasing_level"));
auto& showFPS(lvm.create<bool>("show_fps"));
auto& timerStatic(lvm.create<bool>("timer_static"));
auto& serverLocal(lvm.create<bool>("server_local"));
auto& serverVerbose(lvm.create<bool>("server_verbose"));
auto& mouseVisible(lvm.create<bool>("mouse_visible"));
auto& musicSpeedMult(lvm.create<float>("music_speed_mult"));
auto& drawTextOutlines(lvm.create<bool>("draw_text_outlines"));
auto& darkenUnevenBackgroundChunk(
    lvm.create<bool>("darken_uneven_background_chunk"));
auto& rotateToStart(lvm.create<bool>("rotate_to_start"));
auto& triggerRotateCCW(lvm.create<Trigger>("t_rotate_ccw"));
auto& triggerRotateCW(lvm.create<Trigger>("t_rotate_cw"));
auto& triggerFocus(lvm.create<Trigger>("t_focus"));
auto& triggerExit(lvm.create<Trigger>("t_exit"));
auto& triggerForceRestart(lvm.create<Trigger>("t_force_restart"));
auto& triggerRestart(lvm.create<Trigger>("t_restart"));
auto& triggerScreenshot(lvm.create<Trigger>("t_screenshot"));
auto& triggerSwap(lvm.create<Trigger>("t_swap"));
auto& triggerUp(lvm.create<Trigger>("t_up"));
auto& triggerDown(lvm.create<Trigger>("t_down"));

float sizeX{1500}, sizeY{1500};
constexpr float spawnDistance{1600};
string uneligibilityReason;

void applyAutoWindowedResolution()
{
    auto d(VideoMode::getDesktopMode());
    windowedWidth = d.width;
    windowedHeight = d.height;
}
void applyAutoFullscreenResolution()
{
    auto d(VideoMode::getDesktopMode());
    fullscreenWidth = d.width;
    fullscreenHeight = d.height;
}

void loadConfig(const vector<string>& mOverridesIds)
{
    lo("::loadConfig") << "loading config\n";

    for(const auto& p :
        getScan<ssvufs::Mode::Single, ssvufs::Type::File, ssvufs::Pick::ByExt>(
            "ConfigOverrides/", ".json"))
    {
        if(contains(mOverridesIds, p.getFileNameNoExtensions()))
        {
            const auto& overrideRoot(getFromFile(p));
            for(auto itr(begin(overrideRoot)); itr != end(overrideRoot); ++itr)
                root[getKey(itr)] = *itr;
        }
    }

    lvm.syncFromObj();

    if(getWindowedAutoResolution()) applyAutoWindowedResolution();
    if(getFullscreenAutoResolution()) applyAutoFullscreenResolution();

    recalculateSizes();
}
void saveConfig()
{
    lo("::saveConfig") << "saving config\n";
    lvm.syncToObj();
    writeToFile(root, "config.json");
}

bool isEligibleForScore()
{
    if(!getOfficial())
    {
        uneligibilityReason = "official mode off";
        return false;
    }
    if(getDebug())
    {
        uneligibilityReason = "debug mode on";
        return false;
    }
    if(!getAutoZoomFactor())
    {
        uneligibilityReason = "modified zoom factor";
        return false;
    }
    if(getPlayerSpeed() != 9.45f)
    {
        uneligibilityReason = "player speed modified";
        return false;
    }
    if(getPlayerFocusSpeed() != 4.625f)
    {
        uneligibilityReason = "player focus speed modified";
        return false;
    }
    if(getPlayerSize() != 7.3f)
    {
        uneligibilityReason = "player size modified";
        return false;
    }
    if(getInvincible())
    {
        uneligibilityReason = "invincibility on";
        return false;
    }
    if(getNoRotation())
    {
        uneligibilityReason = "rotation off";
        return false;
    }
    if(Online::getServerVersion() == -1)
    {
        uneligibilityReason = "connection error";
        return false;
    }
    if(Online::getServerVersion() > getVersion())
    {
        uneligibilityReason = "version mismatch";
        return false;
    }
    return true;
}

void recalculateSizes()
{
    sizeX = sizeY = max(getWidth(), getHeight()) * 1.3f;
    if(!getAutoZoomFactor()) return;

    float factorX(1024.f / ssvu::toFloat(getWidth())),
        factorY(768.f / ssvu::toFloat(getHeight()));
    zoomFactor = max(factorX, factorY);
}
void setFullscreen(GameWindow& mWindow, bool mFullscreen)
{
    fullscreen = mFullscreen;

    mWindow.setSize(getWidth(), getHeight());
    mWindow.setFullscreen(getFullscreen());
    mWindow.setMouseCursorVisible(Config::getMouseVisible());

    recalculateSizes();
}

void refreshWindowSize(unsigned int mWidth, unsigned int mHeight)
{
    windowedWidth = mWidth;
    windowedHeight = mHeight;
}

void setCurrentResolution(
    GameWindow& mWindow, unsigned int mWidth, unsigned int mHeight)
{
    if(fullscreen)
    {
        fullscreenAutoResolution = false;
        fullscreenWidth = mWidth;
        fullscreenHeight = mHeight;
    }
    else
    {
        windowedAutoResolution = false;
        windowedWidth = mWidth;
        windowedHeight = mHeight;
    }

    mWindow.setSize(getWidth(), getHeight());
    mWindow.setFullscreen(getFullscreen());
    mWindow.setMouseCursorVisible(Config::getMouseVisible());
    recalculateSizes();
}
void setCurrentResolutionAuto(GameWindow& mWindow)
{
    if(fullscreen)
    {
        fullscreenAutoResolution = true;
        applyAutoFullscreenResolution();
    }
    else
    {
        windowedAutoResolution = true;
        applyAutoWindowedResolution();
    }

    mWindow.setSize(getWidth(), getHeight());
    mWindow.setFullscreen(getFullscreen());
    mWindow.setMouseCursorVisible(Config::getMouseVisible());
    recalculateSizes();
}
void setVsync(GameWindow& mWindow, bool mValue)
{
    vsync = mValue;
    mWindow.setVsync(vsync);
}
void setLimitFPS(GameWindow& mWindow, bool mValue)
{
    limitFPS = mValue;
    mWindow.setFPSLimited(mValue);
}
void setMaxFPS(GameWindow& mWindow, unsigned int mValue)
{
    maxFPS = mValue;
    mWindow.setMaxFPS(mValue);
}
void setTimerStatic(GameWindow& mWindow, bool mValue)
{
    timerStatic = mValue;
    if(timerStatic)
    {
        mWindow.setTimer<TimerStatic>(0.5f, 0.5f);
    }
    else
    {
        mWindow.setTimer<TimerDynamic>();
        setLimitFPS(mWindow, true);
        setMaxFPS(mWindow, 200);
    }
}
void setAntialiasingLevel(GameWindow& mWindow, unsigned int mValue)
{
    antialiasingLevel = mValue;
    mWindow.setAntialiasingLevel(mValue);
}

void setOnline(bool mOnline)
{
    online = mOnline;
}
void setOfficial(bool mOfficial)
{
    official = mOfficial;
}
void setNoRotation(bool mNoRotation)
{
    noRotation = mNoRotation;
}
void setNoBackground(bool mNoBackground)
{
    noBackground = mNoBackground;
}
void setBlackAndWhite(bool mBlackAndWhite)
{
    blackAndWhite = mBlackAndWhite;
}
void setNoSound(bool mNoSound)
{
    noSound = mNoSound;
}
void setNoMusic(bool mNoMusic)
{
    noMusic = mNoMusic;
}
void setPulse(bool mPulse)
{
    pulseEnabled = mPulse;
}
void set3D(bool m3D)
{
    _3DEnabled = m3D;
}
void setInvincible(bool mInvincible)
{
    invincible = mInvincible;
}
void setAutoRestart(bool mAutoRestart)
{
    autoRestart = mAutoRestart;
}
void setSoundVolume(float mVolume)
{
    soundVolume = mVolume;
}
void setMusicVolume(float mVolume)
{
    musicVolume = mVolume;
}
void setFlash(bool mFlash)
{
    flashEnabled = mFlash;
}
void setMusicSpeedDMSync(bool mValue)
{
    musicSpeedDMSync = mValue;
}
void setShowFPS(bool mValue)
{
    showFPS = mValue;
}
void setServerLocal(bool mValue)
{
    serverLocal = mValue;
}
void setServerVerbose(bool mValue)
{
    serverVerbose = mValue;
}
void setMouseVisible(bool mValue)
{
    mouseVisible = mValue;
}
void setMusicSpeedMult(float mValue)
{
    musicSpeedMult = mValue;
}
void setDrawTextOutlines(bool mX)
{
    drawTextOutlines = mX;
}
void setDarkenUnevenBackgroundChunk(bool mX)
{
    darkenUnevenBackgroundChunk = mX;
}
void setRotateToStart(bool mX)
{
    rotateToStart = mX;
}

bool SSVU_ATTRIBUTE(pure) getOnline()
{
    return online;
}
bool SSVU_ATTRIBUTE(pure) getOfficial()
{
    return official;
}
string SSVU_ATTRIBUTE(pure) getUneligibilityReason()
{
    return uneligibilityReason;
}
float SSVU_ATTRIBUTE(pure) getSizeX()
{
    return sizeX;
}
float SSVU_ATTRIBUTE(pure) getSizeY()
{
    return sizeY;
}
float SSVU_ATTRIBUTE(const) getSpawnDistance()
{
    return spawnDistance;
}
float SSVU_ATTRIBUTE(pure) getZoomFactor()
{
    return zoomFactor;
}
int SSVU_ATTRIBUTE(pure) getPixelMultiplier()
{
    return pixelMultiplier;
}
float SSVU_ATTRIBUTE(pure) getPlayerSpeed()
{
    return official ? 9.45f : playerSpeed;
}
float SSVU_ATTRIBUTE(pure) getPlayerFocusSpeed()
{
    return official ? 4.625f : playerFocusSpeed;
}
float SSVU_ATTRIBUTE(pure) getPlayerSize()
{
    return official ? 7.3f : playerSize;
}
bool SSVU_ATTRIBUTE(pure) getNoRotation()
{
    return official ? false : noRotation;
}
bool SSVU_ATTRIBUTE(pure) getNoBackground()
{
    return official ? false : noBackground;
}
bool SSVU_ATTRIBUTE(pure) getBlackAndWhite()
{
    return official ? false : blackAndWhite;
}
bool SSVU_ATTRIBUTE(pure) getNoSound()
{
    return noSound;
}
bool SSVU_ATTRIBUTE(pure) getNoMusic()
{
    return noMusic;
}
float SSVU_ATTRIBUTE(pure) getSoundVolume()
{
    return soundVolume;
}
float SSVU_ATTRIBUTE(pure) getMusicVolume()
{
    return musicVolume;
}
bool SSVU_ATTRIBUTE(pure) getLimitFPS()
{
    return limitFPS;
}
bool SSVU_ATTRIBUTE(pure) getVsync()
{
    return vsync;
}
bool SSVU_ATTRIBUTE(pure) getAutoZoomFactor()
{
    return official ? true : autoZoomFactor;
}
bool SSVU_ATTRIBUTE(pure) getFullscreen()
{
    return fullscreen;
}
float SSVU_ATTRIBUTE(const) getVersion()
{
    return 2.f;
}
bool SSVU_ATTRIBUTE(pure) getWindowedAutoResolution()
{
    return windowedAutoResolution;
}
bool SSVU_ATTRIBUTE(pure) getFullscreenAutoResolution()
{
    return fullscreenAutoResolution;
}
unsigned int SSVU_ATTRIBUTE(pure) getFullscreenWidth()
{
    return fullscreenWidth;
}
unsigned int SSVU_ATTRIBUTE(pure) getFullscreenHeight()
{
    return fullscreenHeight;
}
unsigned int SSVU_ATTRIBUTE(pure) getWindowedWidth()
{
    return windowedWidth;
}
unsigned int SSVU_ATTRIBUTE(pure) getWindowedHeight()
{
    return windowedHeight;
}
unsigned int SSVU_ATTRIBUTE(pure) getWidth()
{
    return fullscreen ? fullscreenWidth : windowedWidth;
}
unsigned int SSVU_ATTRIBUTE(pure) getHeight()
{
    return fullscreen ? fullscreenHeight : windowedHeight;
}
bool SSVU_ATTRIBUTE(pure) getShowMessages()
{
    return showMessages;
}
bool SSVU_ATTRIBUTE(pure) getDebug()
{
    return debug;
}
bool SSVU_ATTRIBUTE(pure) getPulse()
{
    return official ? true : pulseEnabled;
}
bool SSVU_ATTRIBUTE(pure) getBeatPulse()
{
    return official ? true : beatPulse;
}
bool SSVU_ATTRIBUTE(pure) getInvincible()
{
    return official ? false : invincible;
}
bool SSVU_ATTRIBUTE(pure) get3D()
{
    return _3DEnabled;
}
float SSVU_ATTRIBUTE(pure) get3DMultiplier()
{
    return _3DMultiplier;
}
unsigned int SSVU_ATTRIBUTE(pure) get3DMaxDepth()
{
    return _3DMaxDepth;
}
bool SSVU_ATTRIBUTE(pure) getAutoRestart()
{
    return autoRestart;
}
bool SSVU_ATTRIBUTE(pure) getFlash()
{
    return flashEnabled;
}
bool SSVU_ATTRIBUTE(pure) getShowTrackedVariables()
{
    return showTrackedVariables;
}
bool SSVU_ATTRIBUTE(pure) getMusicSpeedDMSync()
{
    return musicSpeedDMSync;
}
unsigned int SSVU_ATTRIBUTE(pure) getMaxFPS()
{
    return maxFPS;
}
unsigned int SSVU_ATTRIBUTE(pure) getAntialiasingLevel()
{
    return antialiasingLevel;
}
bool SSVU_ATTRIBUTE(pure) getShowFPS()
{
    return showFPS;
}
bool SSVU_ATTRIBUTE(pure) getTimerStatic()
{
    return timerStatic;
}
bool SSVU_ATTRIBUTE(pure) getServerLocal()
{
    return serverLocal;
}
bool SSVU_ATTRIBUTE(pure) getServerVerbose()
{
    return serverVerbose;
}
bool SSVU_ATTRIBUTE(pure) getMouseVisible()
{
    return mouseVisible;
}
float SSVU_ATTRIBUTE(pure) getMusicSpeedMult()
{
    return musicSpeedMult;
}
bool SSVU_ATTRIBUTE(pure) getDrawTextOutlines()
{
    return drawTextOutlines;
}
bool SSVU_ATTRIBUTE(pure) getDarkenUnevenBackgroundChunk()
{
    return darkenUnevenBackgroundChunk;
}
bool SSVU_ATTRIBUTE(pure) getRotateToStart()
{
    return rotateToStart;
}

Trigger getTriggerRotateCCW()
{
    return triggerRotateCCW;
}
Trigger getTriggerRotateCW()
{
    return triggerRotateCW;
}
Trigger getTriggerFocus()
{
    return triggerFocus;
}
Trigger getTriggerExit()
{
    return triggerExit;
}
Trigger getTriggerForceRestart()
{
    return triggerForceRestart;
}
Trigger getTriggerRestart()
{
    return triggerRestart;
}
Trigger getTriggerScreenshot()
{
    return triggerScreenshot;
}
Trigger getTriggerSwap()
{
    return triggerSwap;
}
Trigger getTriggerUp()
{
    return triggerUp;
}
Trigger getTriggerDown()
{
    return triggerDown;
}
} // namespace Config
} // namespace hg

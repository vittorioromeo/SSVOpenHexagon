// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Online/Online.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Core/BindControl.hpp"

#include <SSVStart/Input/Input.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SSVMenuSystem/SSVMenuSystem.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Input;
using namespace ssvms;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{

using ocs = Online::ConnectStat;
using ols = Online::LoginStat;

MenuGame::MenuGame(Steam::steam_manager& mSteamManager,
    Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
    HexagonGame& mHexagonGame, GameWindow& mGameWindow)
    : steamManager(mSteamManager), discordManager(mDiscordManager),
      assets(mAssets), hexagonGame(mHexagonGame), window(mGameWindow),
      dialogBox(mAssets, mGameWindow, styleData)
{
    // TODO: check `Config::getFirstTimePlaying` and react accordingly
    Config::setFirstTimePlaying(false);

    initAssets();
    refreshCamera();

    game.onUpdate += [this](ssvu::FT mFT) { update(mFT); };

    game.onDraw += [this] { draw(); };

    game.onEvent(Event::EventType::TextEntered) += [this](const Event& mEvent) {
        if(mEvent.text.unicode < 128)
        {
            enteredChars.emplace_back(toNum<char>(mEvent.text.unicode));
        }
    };

    game.onEvent(Event::EventType::MouseWheelMoved) +=
        [this](const Event& mEvent) {
            wheelProgress += mEvent.mouseWheel.delta;
            if(wheelProgress > 2.f)
            {
                wheelProgress = 0.f;
                upAction();
            }
            else if(wheelProgress < -2.f)
            {
                wheelProgress = 0.f;
                downAction();
            }
        };

    const auto checkCloseDialogBox = [this]() {
        if(!(--noActions))
        {
            dialogBox.clearDialogBox();
            game.ignoreAllInputs(false);
            hg::Joystick::ignoreAllPresses(false);
        }
    };

    game.onEvent(Event::EventType::KeyReleased) +=
        [this, checkCloseDialogBox](const Event& mEvent) {
            // don't do anything if inputs are being processed as usual
            if(!noActions)
            {
                return;
            }

            // Scenario one: actions are blocked cause a dialog box is open
            if(!dialogBox.empty())
            {
                checkCloseDialogBox();
                return;
            }

            // Scenario two: actions are blocked cause we are using a
            // BindControl menu item
            KKey key = mEvent.key.code;
            if(getCurrentMenu() != nullptr && key == KKey::Escape)
            {
                getCurrentMenu()->getItem().exec(); // turn off bind inputting
                game.ignoreAllInputs(false);
                hg::Joystick::ignoreAllPresses(false);
                noActions = 0;
                return;
            }

            if(!(--noActions))
            {
                dialogBox.clearDialogBox();

                if(getCurrentMenu() == nullptr)
                {
                    return;
                }

                auto* bc = dynamic_cast<KeyboardBindControl*>(
                    &getCurrentMenu()->getItem());

                // don't try assigning a keyboard key to a controller bind
                if(bc == nullptr)
                {
                    assets.playSound("error.ogg");
                    noActions = 1;
                    return;
                }

                if(!isValidKeyBind(key))
                {
                    assets.playSound("error.ogg");
                    noActions = 1;
                }
                else
                {
                    bc->newKeyboardBind(key);
                    game.ignoreAllInputs(false);
                    hg::Joystick::ignoreAllPresses(false);
                    assets.playSound("beep.ogg");
                }

                touchDelay = 10.f;
            }
        };

    game.onEvent(Event::EventType::MouseButtonReleased) +=
        [this, checkCloseDialogBox](const Event& mEvent) {
            if(!noActions)
            {
                return;
            }

            if(!dialogBox.empty())
            {
                checkCloseDialogBox();
                return;
            }

            if(!(--noActions))
            {
                if(getCurrentMenu() == nullptr)
                {
                    return;
                }

                auto* bc = dynamic_cast<KeyboardBindControl*>(
                    &getCurrentMenu()->getItem());

                // don't try assigning a keyboard key to a controller bind
                if(bc == nullptr)
                {
                    assets.playSound("error.ogg");
                    noActions = 1;
                    return;
                }

                bc->newKeyboardBind(KKey::Unknown, mEvent.mouseButton.button);
                game.ignoreAllInputs(false);
                hg::Joystick::ignoreAllPresses(false);
                assets.playSound("beep.ogg");
                touchDelay = 10.f;
            }
        };

    game.onEvent(Event::EventType::JoystickButtonReleased) +=
        [this, checkCloseDialogBox](const Event& mEvent) {
            if(!noActions)
            {
                return;
            }

            if(!dialogBox.empty())
            {
                checkCloseDialogBox();
                return;
            }

            // close dialogbox after the second key release
            if(!(--noActions))
            {
                if(getCurrentMenu() == nullptr)
                {
                    return;
                }

                auto* bc = dynamic_cast<JoystickBindControl*>(
                    &getCurrentMenu()->getItem());

                // don't try assigning a controller button to a keyboard bind
                if(bc == nullptr)
                {
                    assets.playSound("error.ogg");
                    noActions = 1;
                    return;
                }

                bc->newJoystickBind(int(mEvent.joystickButton.button));
                game.ignoreAllInputs(false);
                hg::Joystick::ignoreAllPresses(false);
                assets.playSound("beep.ogg");
                touchDelay = 10.f;
            }
        };

    window.onRecreation += [this] { refreshCamera(); };

    initMenus();
    initInput();

    levelDataIds =
        assets.getLevelIdsByPack(assets.getPackInfos().at(packIdx).id);
    setIndex(0);
}

bool MenuGame::loadCommandLineLevel(
    const std::string& pack, const std::string& level)
{
    // First find the ID of the pack with name matching the one typed by the
    // user. `packDatas` is the only vector in assets with a data type
    // containing the name of the pack (without it being part of the id).
    std::string packID;
    for(auto& d : assets.getPacksData())
    {
        if(d.second.name == pack)
        {
            packID = d.second.id;
            break;
        }
    }

    if(packID.empty())
    {
        ssvu::lo("hg::Menugame::MenuGame()")
            << "Invalid pack name '" << pack
            << "' command line parameter, aborting boot level load\n ";

        return false;
    }

    // Iterate through packInfos to find the menu pack index and the index
    // of the level.
    const std::string levelID = packID + "_" + level;
    const auto& p = assets.getPackInfos();
    const auto& levelsList = assets.getLevelIdsByPack(packID);

    for(int i = 0; i < int(p.size()); ++i)
    {
        // once you find the pack index search if it contains the level
        if(packID != p.at(i).id)
        {
            continue;
        }

        auto it = std::find(levelsList.begin(), levelsList.end(), levelID);
        if(it == levelsList.end())
        {

            ssvu::lo("hg::Menugame::MenuGame()")
                << "Invalid level name '" << level
                << "' command line parameter, aborting boot level load\n";

            return false;
        }

        // Level found, initialize parameters
        packIdx = i;
        levelDataIds = levelsList;
        setIndex(it - levelsList.begin());

        break;
    }

    // Do the sequence of actions user would have
    // to do manually to get to the desired level

    playLocally(); // go to profile selection screen

    if(state == States::ETLPNew)
    {
        ssvu::lo("hg::Menugame::MenuGame()")
            << "No player profiles exist, aborting boot level load\n";

        return false;
    }

    // Go to main menu
    enteredStr = ssvu::getByModIdx(assets.getLocalProfileNames(), profileIdx);
    assets.pSetCurrent(enteredStr);
    state = States::SMain;

    // Start game
    window.setGameState(hexagonGame.getGame());
    hexagonGame.newGame(packID, levelDataIds.at(currentIndex), true,
        ssvu::getByModIdx(diffMults, diffMultIdx), false);

    return true;
}

void MenuGame::init(bool error)
{
    steamManager.set_rich_presence_in_menu();
    steamManager.update_hardcoded_achievements();

    discordManager.set_rich_presence_in_menu();

    assets.stopMusics();
    assets.stopSounds();
    if(!error)
    {
        assets.playSound("openHexagon.ogg");
    }
    else
    {
        assets.playSound("error.ogg");
    }

    Online::setForceLeaderboardRefresh(true);
}

void MenuGame::init(
    bool error, const std::string& pack, const std::string& level)
{
    steamManager.set_rich_presence_in_menu();
    steamManager.update_hardcoded_achievements();

    discordManager.set_rich_presence_in_menu();

    assets.stopMusics();
    assets.stopSounds();

    if(!error)
    {
        assets.playSound("openHexagon.ogg");
    }
    else
    {
        assets.playSound("error.ogg");
    }

    Online::setForceLeaderboardRefresh(true);

    loadCommandLineLevel(pack, level);
}

void MenuGame::initAssets()
{
    for(const auto& t : {"titleBar.png", "creditsBar1.png", "creditsBar2.png",
            "creditsBar2b.png", "creditsBar2c.png", "creditsBar2d.png",
            "bottomBar.png", "epilepsyWarning.png"})
    {
        assets.get<Texture>(t).setSmooth(true);
    }
}

void MenuGame::playLocally()
{
    assets.pSaveCurrent();
    assets.pSetPlayingLocally(true);
    enteredStr = "";
    state = assets.getLocalProfilesSize() == 0 ? States::ETLPNew
                                               : States::SLPSelect;
}

void MenuGame::initMenus()
{
    namespace i = ssvms::Items;

    auto whenLocal = [this] { return assets.pIsLocal(); };
    auto whenNotLocal = [this] { return !assets.pIsLocal(); };
    auto whenNotOfficial = [] { return !Config::getOfficial(); };
    auto whenDisconnected = [] {
        return Online::getConnectionStatus() == ocs::Disconnected;
    };
    auto whenConnectedAndUnlogged = [] {
        return Online::getConnectionStatus() == ocs::Connected &&
               Online::getLoginStatus() == ols::Unlogged;
    };
    auto whenConnectedAndLogged = [] {
        return Online::getConnectionStatus() == ocs::Connected &&
               Online::getLoginStatus() == ols::Logged;
    };
    auto whenUnlogged = [] {
        return Online::getLoginStatus() == ols::Unlogged;
    };
    auto whenSoundEnabled = [] { return !Config::getNoSound(); };
    auto whenMusicEnabled = [] { return !Config::getNoMusic(); };
    auto whenTimerIsStatic = [] { return Config::getTimerStatic(); };
    auto whenTimerIsDynamic = [] { return !Config::getTimerStatic(); };

    // Welcome menu
    auto& wlcm(welcomeMenu.createCategory("welcome"));
    wlcm.create<i::Single>("connect", [] { Online::tryConnectToServer(); }) |
        whenDisconnected;
    wlcm.create<i::Single>("login", [this] {
        assets.pSaveCurrent();
        assets.pSetPlayingLocally(false);
        enteredStr = "";
        state = States::ETUser;
    }) | whenConnectedAndUnlogged;
    wlcm.create<i::Single>("logout", [] { Online::logout(); }) |
        whenConnectedAndLogged;
    wlcm.create<i::Single>("play locally", [this] { playLocally(); }) |
        whenUnlogged;
    wlcm.create<i::Single>("exit game", [this] { window.stop(); });

    // Options menu
    auto& main(optionsMenu.createCategory("options"));
    auto& friends(optionsMenu.createCategory("friends"));
    auto& play(optionsMenu.createCategory("gameplay"));
    auto& keyboard(optionsMenu.createCategory("keyboard"));
    auto& joystick(optionsMenu.createCategory("joystick"));
    auto& resolution(optionsMenu.createCategory("resolution"));
    auto& gfx(optionsMenu.createCategory("graphics"));
    auto& sfx(optionsMenu.createCategory("audio"));
    auto& debug(optionsMenu.createCategory("debug"));
    auto& localProfiles(optionsMenu.createCategory("local profiles"));

    main.create<i::Goto>("friends", friends) | whenNotLocal;
    main.create<i::Goto>("gameplay", play);
    main.create<i::Goto>("resolution", resolution);
    main.create<i::Goto>("graphics", gfx);
    main.create<i::Goto>("audio", sfx);
    main.create<i::Goto>("debug", debug) | whenNotOfficial;
    main.create<i::Goto>("local profiles", localProfiles) | whenLocal;

    // TODO:
    // main.create<i::Single>("login screen", [this] { state = States::MWlcm;
    // });
    // main.create<i::Toggle>("online", &Config::getOnline, &Config::setOnline);

    main.create<i::Toggle>(
        "official mode", &Config::getOfficial, &Config::setOfficial);
    main.create<i::Single>("exit game", [this] { window.stop(); });
    main.create<i::Single>("back", [this] { state = States::SMain; });


    // Resolution
    resolution.create<i::Single>(
        "auto", [this] { Config::setCurrentResolutionAuto(window); });

    for(const auto& vm : VideoMode::getFullscreenModes())
    {
        if(vm.bitsPerPixel == 32)
        {
            resolution.create<i::Single>(
                toStr(vm.width) + "x" + toStr(vm.height), [this, &vm] {
                    Config::setCurrentResolution(window, vm.width, vm.height);
                });
        }
    }

    resolution.create<i::Single>(
        "go windowed", [this] { Config::setFullscreen(window, false); });
    resolution.create<i::Single>(
        "go fullscreen", [this] { Config::setFullscreen(window, true); });
    resolution.create<i::GoBack>("back");


    // Graphics
    gfx.create<i::Toggle>("3D effects", &Config::get3D, &Config::set3D);
    gfx.create<i::Toggle>(
        "no rotation", &Config::getNoRotation, &Config::setNoRotation) |
        whenNotOfficial;
    gfx.create<i::Toggle>(
        "no background", &Config::getNoBackground, &Config::setNoBackground) |
        whenNotOfficial;
    gfx.create<i::Toggle>(
        "b&w colors", &Config::getBlackAndWhite, &Config::setBlackAndWhite) |
        whenNotOfficial;
    gfx.create<i::Toggle>("pulse", &Config::getPulse, &Config::setPulse) |
        whenNotOfficial;

    gfx.create<i::Toggle>("flash", &Config::getFlash, &Config::setFlash);
    gfx.create<i::Toggle>("vsync", &Config::getVsync,
        [this](bool mValue) { Config::setVsync(window, mValue); });
    gfx.create<i::Single>(
        "go windowed", [this] { Config::setFullscreen(window, false); });
    gfx.create<i::Single>(
        "go fullscreen", [this] { Config::setFullscreen(window, true); });

    gfx.create<i::Single>("use static fps", [this] {
        Config::setTimerStatic(window, true);
    }) | whenTimerIsDynamic;
    gfx.create<i::Single>("use dynamic fps", [this] {
        Config::setTimerStatic(window, false);
    }) | whenTimerIsStatic;

    gfx.create<i::Toggle>("limit fps", &Config::getLimitFPS,
        [this](bool mValue) { Config::setLimitFPS(window, mValue); }) |
        whenTimerIsStatic;
    gfx.create<i::Slider>(
        "max fps", &Config::getMaxFPS,
        [this](unsigned int mValue) { Config::setMaxFPS(window, mValue); }, 30u,
        200u, 5u) |
        whenTimerIsStatic;
    gfx.create<i::Slider>(
        "antialiasing", &Config::getAntialiasingLevel,
        [this](unsigned int mValue) {
            Config::setAntialiasingLevel(window, mValue);
        },
        0u, 3u, 1u);
    gfx.create<i::Toggle>("show fps", &Config::getShowFPS, &Config::setShowFPS);
    gfx.create<i::Toggle>("text outlines", &Config::getDrawTextOutlines,
        &Config::setDrawTextOutlines);
    gfx.create<i::Toggle>("darken uneven background chunk",
        &Config::getDarkenUnevenBackgroundChunk,
        &Config::setDarkenUnevenBackgroundChunk);
    gfx.create<i::Slider>(
        "text padding", &Config::getTextPadding,
        [this](float mValue) { Config::setTextPadding(mValue); }, 0.f, 64.f,
        1.f);
    gfx.create<i::Slider>(
        "text scaling", &Config::getTextScaling,
        [this](float mValue) { Config::setTextScaling(mValue); }, 0.1f, 4.f,
        0.05f);
    gfx.create<i::Toggle>(
        "show key icons", &Config::getShowKeyIcons, &Config::setShowKeyIcons);
    gfx.create<i::Slider>(
        "key icons scaling", &Config::getKeyIconsScale,
        [this](float mValue) { Config::setKeyIconsScale(mValue); }, 0.1f, 4.f,
        0.05f);
    gfx.create<i::GoBack>("back");


    // Sound
    sfx.create<i::Toggle>("no sound", &Config::getNoSound, &Config::setNoSound);
    sfx.create<i::Toggle>("no music", &Config::getNoMusic, &Config::setNoMusic);
    sfx.create<i::Slider>(
        "sound volume", &Config::getSoundVolume,
        [this](unsigned int mValue) {
            Config::setSoundVolume(mValue);
            assets.refreshVolumes();
        },
        0u, 100u, 5u) |
        whenSoundEnabled;
    sfx.create<i::Slider>(
        "music volume", &Config::getMusicVolume,
        [this](unsigned int mValue) {
            Config::setMusicVolume(mValue);
            assets.refreshVolumes();
        },
        0u, 100u, 5u) |
        whenMusicEnabled;
    sfx.create<i::Toggle>("sync music with difficulty",
        &Config::getMusicSpeedDMSync, &Config::setMusicSpeedDMSync) |
        whenMusicEnabled;
    sfx.create<i::Slider>(
        "music speed multiplier", &Config::getMusicSpeedMult,
        [](float mValue) { Config::setMusicSpeedMult(mValue); }, 0.7f, 1.3f,
        0.05f) |
        whenMusicEnabled;
    sfx.create<i::GoBack>("back");


    // Gameplay
    play.create<i::Toggle>(
        "autorestart", &Config::getAutoRestart, &Config::setAutoRestart);
    play.create<i::Toggle>("rotate to start", &Config::getRotateToStart,
        &Config::setRotateToStart);
    play.create<i::Goto>("keyboard", keyboard);
    play.create<i::Goto>("joystick", joystick);
    play.create<i::GoBack>("back");

    // Keyboard binds
    auto callBack = [this](const Trigger& trig, const int bindID) {
        game.refreshTrigger(trig, bindID);
        hexagonGame.refreshTrigger(trig, bindID);
    };

    keyboard.create<KeyboardBindControl>("rotate ccw",
        &Config::getTriggerRotateCCW, &Config::reassignBindTriggerRotateCCW,
        &Config::clearBindTriggerRotateCCW, callBack, Tid::RotateCCW);
    keyboard.create<KeyboardBindControl>("rotate cw",
        &Config::getTriggerRotateCW, &Config::reassignBindTriggerRotateCW,
        &Config::clearBindTriggerRotateCW, callBack, Tid::RotateCW);
    keyboard.create<KeyboardBindControl>("focus", &Config::getTriggerFocus,
        &Config::reassignBindTriggerFocus, &Config::clearBindTriggerFocus,
        callBack, Tid::Focus);
    keyboard.create<KeyboardBindControl>("exit", &Config::getTriggerExit,
        &Config::reassignBindTriggerExit, &Config::clearBindTriggerExit,
        callBack, Tid::Exit);
    keyboard.create<KeyboardBindControl>("force restart",
        &Config::getTriggerForceRestart,
        &Config::reassignBindTriggerForceRestart,
        &Config::clearBindTriggerForceRestart, callBack, Tid::ForceRestart);
    keyboard.create<KeyboardBindControl>("restart", &Config::getTriggerRestart,
        &Config::reassignBindTriggerRestart, &Config::clearBindTriggerRestart,
        callBack, Tid::Restart);
    keyboard.create<KeyboardBindControl>("replay", &Config::getTriggerReplay,
        &Config::reassignBindTriggerReplay, &Config::clearBindTriggerReplay,
        callBack, Tid::Replay);
    keyboard.create<KeyboardBindControl>("screenshot",
        &Config::getTriggerScreenshot, &Config::reassignBindTriggerScreenshot,
        &Config::clearBindTriggerScreenshot, callBack, Tid::Screenshot);
    keyboard.create<KeyboardBindControl>("swap", &Config::getTriggerSwap,
        &Config::reassignBindTriggerSwap, &Config::clearBindTriggerSwap,
        callBack, Tid::Swap);
    keyboard.create<KeyboardBindControl>("up", &Config::getTriggerUp,
        &Config::reassignBindTriggerUp, &Config::clearBindTriggerUp, callBack,
        Tid::Up);
    keyboard.create<KeyboardBindControl>("down", &Config::getTriggerDown,
        &Config::reassignBindTriggerDown, &Config::clearBindTriggerDown,
        callBack, Tid::Down);
    keyboard.create<i::GoBack>("back");

    // Joystick binds
    joystick.create<i::Slider>("joystick deadzone",
        &Config::getJoystickDeadzone, &Config::setJoystickDeadzone, 0.f, 100.f,
        1.f);

    using Jid = hg::Joystick::Jid;

    auto JoystickCallBack = [this](
                                const unsigned int button, const int buttonID) {
        hg::Joystick::setJoystickBind(button, buttonID);
    };

    joystick.create<JoystickBindControl>("select", &Config::getJoystickSelect,
        &Config::reassignToJoystickSelect, JoystickCallBack, Jid::Select);
    joystick.create<JoystickBindControl>("exit", &Config::getJoystickExit,
        &Config::reassignToJoystickExit, JoystickCallBack, Jid::Exit);
    joystick.create<JoystickBindControl>("focus", &Config::getJoystickFocus,
        &Config::reassignToJoystickFocus, JoystickCallBack, Jid::Focus);
    joystick.create<JoystickBindControl>("swap", &Config::getJoystickSwap,
        &Config::reassignToJoystickSwap, JoystickCallBack, Jid::Swap);
    joystick.create<JoystickBindControl>("force restart",
        &Config::getJoystickForceRestart,
        &Config::reassignToJoystickForceRestart, JoystickCallBack,
        Jid::ForceRestart);
    joystick.create<JoystickBindControl>("restart", &Config::getJoystickRestart,
        &Config::reassignToJoystickRestart, JoystickCallBack, Jid::Restart);
    joystick.create<JoystickBindControl>("replay", &Config::getJoystickReplay,
        &Config::reassignToJoystickReplay, JoystickCallBack, Jid::Replay);
    joystick.create<JoystickBindControl>("screenshot",
        &Config::getJoystickScreenshot, &Config::reassignToJoystickScreenshot,
        JoystickCallBack, Jid::Screenshot);
    joystick.create<JoystickBindControl>("option menu",
        &Config::getJoystickOptionMenu, &Config::reassignToJoystickOptionMenu,
        JoystickCallBack, Jid::OptionMenu);
    joystick.create<JoystickBindControl>("change pack",
        &Config::getJoystickChangePack, &Config::reassignToJoystickChangePack,
        JoystickCallBack, Jid::ChangePack);
    joystick.create<JoystickBindControl>("create profile",
        &Config::getJoystickCreateProfile,
        &Config::reassignToJoystickCreateProfile, JoystickCallBack,
        Jid::CreateProfile);
    joystick.create<i::GoBack>("back");


    // Profile
    localProfiles.create<i::Single>("change local profile", [this] {
        enteredStr = "";
        state = States::SLPSelect;
    });
    localProfiles.create<i::Single>("new local profile", [this] {
        enteredStr = "";
        state = States::SLPSelect;
    });
    localProfiles.create<i::GoBack>("back");


    // Debug
    debug.create<i::Toggle>("debug mode", &Config::getDebug, &Config::setDebug);
    debug.create<i::Toggle>(
        "invincible", &Config::getInvincible, &Config::setInvincible);
    debug.create<i::Slider>("timescale", &Config::getTimescale,
        &Config::setTimescale, 0.1f, 2.f, 0.05f);
    debug.create<i::GoBack>("back");


    // Friends
    friends.create<i::Single>("add friend", [this] {
        enteredStr = "";
        state = States::ETFriend;
    });
    friends.create<i::Single>(
        "clear friends", [this] { assets.pClearTrackedNames(); });
    friends.create<i::GoBack>("back");
}

void MenuGame::leftAction()
{
    assets.playSound("beep.ogg");
    touchDelay = 50.f;

    if(state == States::EpilepsyWarning)
    {
        // TODO: remove when welcome screen is implemented
        playLocally();
    }
    else if(state == States::SLPSelect)
    {
        --profileIdx;
    }
    else if(state == States::SMain)
    {
        setIndex(currentIndex - 1);
    }
    else if(isInMenu())
    {
        getCurrentMenu()->decrease();
    }
}

void MenuGame::rightAction()
{
    assets.playSound("beep.ogg");
    touchDelay = 50.f;

    if(state == States::EpilepsyWarning)
    {
        // TODO: remove when welcome screen is implemented
        playLocally();
    }
    else if(state == States::SLPSelect)
    {
        ++profileIdx;
    }
    else if(state == States::SMain)
    {
        setIndex(currentIndex + 1);
    }
    else if(isInMenu())
    {
        getCurrentMenu()->increase();
    }
}
void MenuGame::upAction()
{
    assets.playSound("beep.ogg");
    touchDelay = 50.f;

    if(state == States::EpilepsyWarning)
    {
        // TODO: remove when welcome screen is implemented
        playLocally();
    }
    else if(state == States::SMain)
    {
        ++diffMultIdx;
    }
    else if(isInMenu())
    {
        getCurrentMenu()->previous();
    }
}
void MenuGame::downAction()
{
    assets.playSound("beep.ogg");
    touchDelay = 50.f;

    if(state == States::EpilepsyWarning)
    {
        // TODO: remove when welcome screen is implemented
        playLocally();
    }
    else if(state == States::SMain)
    {
        --diffMultIdx;
    }
    else if(isInMenu())
    {
        getCurrentMenu()->next();
    }
}
void MenuGame::okAction()
{
    assets.playSound("select.ogg");
    touchDelay = 50.f;

    if(state == States::EpilepsyWarning)
    {
        // TODO: remove when welcome screen is implemented
        playLocally();
    }
    else if(state == States::SLPSelect)
    {
        assets.pSetCurrent(enteredStr);
        state = States::SMain;
    }
    else if(state == States::SMain)
    {
        window.setGameState(hexagonGame.getGame());
        const std::string& packId = assets.getPackInfos().at(packIdx).id;
        hexagonGame.newGame(packId, levelDataIds.at(currentIndex), true,
            ssvu::getByModIdx(diffMults, diffMultIdx),
            false /* executeLastReplay */);
    }
    else if(isInMenu())
    {
        getCurrentMenu()->exec();

        if(state != States::MOpts)
        {
            return;
        }

        // There are two Bind controllers: KeyboardBindControl and
        // JoystickBindControl. So we cast to the common base class to not check
        // for one and the other.
        auto* bc = dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem());
        if(bc == nullptr || !bc->isWaitingForBind())
        {
            return;
        }

        noActions = 2;
        game.ignoreAllInputs(true);
        hg::Joystick::ignoreAllPresses(true);
        touchDelay = 10.f;
    }
    else if(state == States::ETLPNew)
    {
        if(!enteredStr.empty())
        {
            assets.pCreate(enteredStr);
            assets.pSetCurrent(enteredStr);
            state = States::SMain;
            enteredStr = "";
        }
    }
    else if(state == States::ETFriend)
    {
        if(!enteredStr.empty() &&
            !ssvu::contains(assets.pGetTrackedNames(), enteredStr))
        {
            assets.pAddTrackedName(enteredStr);
            state = States::SMain;
            enteredStr = "";
        }
    }
    else if(state == States::ETUser)
    {
        if(!enteredStr.empty())
        {
            lrUser = enteredStr;
            state = States::ETPass;
            enteredStr = "";
        }
    }
    else if(state == States::ETPass)
    {
        if(!enteredStr.empty())
        {
            lrPass = enteredStr;
            state = States::SLogging;
            enteredStr = "";
            Online::tryLogin(lrUser, lrPass);
        }
    }
    else if(state == States::ETEmail)
    {
        if(!enteredStr.empty() && ssvu::contains(enteredStr, '@'))
        {
            lrEmail = enteredStr;
            enteredStr = "";
            Online::trySendUserEmail(lrEmail);
        }
    }
}

void MenuGame::eraseAction()
{
    if(isEnteringText() && !enteredStr.empty())
    {
        enteredStr.erase(enteredStr.end() - 1);
    }
    else if(state == States::MOpts && isInMenu())
    {
        auto* bc = dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem());
        if(bc == nullptr)
        {
            return;
        }

        if(bc->erase())
        {
            assets.playSound("beep.ogg");
        }
        touchDelay = 10.f;
    }
}

void MenuGame::exitAction()
{
    assets.playSound("beep.ogg");

    if((assets.pIsLocal() && assets.pIsValidLocalProfile()) ||
        !assets.pIsLocal())
    {
        if(isInMenu())
        {
            if(getCurrentMenu()->canGoBack())
            {
                getCurrentMenu()->goBack();
            }
            else
            {
                state = States::SMain;
            }
        }
        else if(state == States::ETFriend || state == States::SLPSelect)
        {
            state = States::SMain;
        }
    }
}

void MenuGame::createProfileAction()
{
    assets.playSound("beep.ogg");
    if(!assets.pIsLocal())
    {
        state = States::MWlcm;
        return;
    }
    if(state == States::SLPSelect)
    {
        enteredStr = "";
        state = States::ETLPNew;
    }
}

void MenuGame::selectProfileAction()
{
    if(state != States::SMain)
    {
        return;
    }
    if(!assets.pIsLocal())
    {
        state = States::MWlcm;
        return;
    }
    assets.playSound("select.ogg");
    enteredStr = "";
    state = States::SLPSelect;
}

void MenuGame::openOptionsAction()
{
    if(state != States::SMain)
    {
        return;
    }
    assets.playSound("select.ogg");
    state = States::MOpts;
}

void MenuGame::selectPackAction()
{
    assets.playSound("beep.ogg");
    if(state == States::SMain)
    {
        const auto& p(assets.getPackInfos());
        packIdx = ssvu::getMod(packIdx + 1, p.size());
        levelDataIds = assets.getLevelIdsByPack(p.at(packIdx).id);
        setIndex(0);
    }
}


void MenuGame::initInput()
{
    using k = KKey;
    using t = Type;

    game.addInput(
        Config::getTriggerRotateCCW(),
        [this](ssvu::FT /*unused*/) { leftAction(); }, t::Once, Tid::RotateCCW);

    game.addInput(
        Config::getTriggerRotateCW(),
        [this](ssvu::FT /*unused*/) { rightAction(); }, t::Once, Tid::RotateCW);

    game.addInput(
        {{k::Up}}, [this](ssvu::FT /*unused*/) { upAction(); }, // hardcoded
        t::Once);

    game.addInput(
        Config::getTriggerUp(),
        [this](ssvu::FT /*unused*/) { upAction(); }, // editable
        t::Once, Tid::Up);

    game.addInput(
        {{k::Down}}, [this](ssvu::FT /*unused*/) { downAction(); }, // hardcoded
        t::Once);

    game.addInput(
        Config::getTriggerDown(),
        [this](ssvu::FT /*unused*/) { downAction(); }, // editable
        t::Once, Tid::Down);

    game.addInput(
        {{k::Return}}, [this](ssvu::FT /*unused*/) { okAction(); }, t::Once);

    game.addInput(
        {{k::F1}}, [this](ssvu::FT /*unused*/) { createProfileAction(); },
        t::Once);

    game.addInput(
        {{k::F2}, {k::J}},
        [this](ssvu::FT /*unused*/) { selectProfileAction(); }, t::Once);

    game.addInput(
        {{k::F3}, {k::K}}, [this](ssvu::FT /*unused*/) { openOptionsAction(); },
        t::Once);

    game.addInput(
        {{k::F4}, {k::L}}, [this](ssvu::FT /*unused*/) { selectPackAction(); },
        t::Once);

    const auto handleExitInput = [this](ssvu::FT /*unused*/) { exitAction(); };

    game.addInput(
        {{k::Escape}},
        [this](ssvs::FT mFT) {
            if(state != States::MOpts)
            {
                exitTimer += mFT;
            }
        }, // hardcoded
        [this](ssvu::FT /*unused*/) { exitTimer = 0; }, t::Always);

    game.addInput({{k::Escape}},
        handleExitInput, // hardcoded
        t::Once);

    game.addInput(Config::getTriggerExit(), handleExitInput, t::Once,
        Tid::Exit); // editable

    game.addInput(
        Config::getTriggerScreenshot(),
        [this](ssvu::FT /*unused*/) { mustTakeScreenshot = true; }, t::Once,
        Tid::Screenshot);

    game.addInput(
            {{k::LAlt, k::Return}},
            [this](ssvu::FT /*unused*/) {
                Config::setFullscreen(window, !window.getFullscreen());
                game.ignoreNextInputs();
            },
            t::Once)
        .setPriorityUser(-1000);

    game.addInput(
        {{k::BackSpace}}, [this](ssvu::FT /*unused*/) { eraseAction(); },
        t::Once);

    game.addInput(
        {{k::F5}}, [this](ssvu::FT /*unused*/) { reloadLevelAssets(); },
        t::Once);
}

void MenuGame::reloadLevelAssets()
{
    if(state != States::SMain || !dialogBox.empty() || !Config::getDebug())
    {
        return;
    }

    // needs to be two because the dialog box reacts to key releases.
    // First key release is the one of the key press that made the dialog
    // box pop up, the second one belongs to the key press that closes it
    noActions = 2;
    assets.playSound("beep.ogg");

    auto [success, reloadOutput] = assets.reloadLevelData(
        levelData->packId, levelData->packPath, levelData->id);

    if(success)
    {
        setIndex(currentIndex); // loads the new levelData

        reloadOutput += assets.reloadMusicData(
            levelData->packId, levelData->packPath, levelData->musicId);

        reloadOutput += assets.reloadStyleData(
            levelData->packId, levelData->packPath, levelData->styleId);

        if(levelData->musicId != "nullMusicId")
        {
            reloadOutput += assets.reloadMusic(
                levelData->packId, levelData->packPath, levelData->musicId);
        }

        if(levelData->soundId != "nullSoundId")
        {
            reloadOutput += assets.reloadCustomSounds(
                levelData->packId, levelData->packPath, levelData->soundId);
        }
    }

    reloadOutput += "\npress any key to close this message\n";
    Utils::uppercasify(reloadOutput);

    dialogBox.createDialogBox(reloadOutput, 26);
    game.ignoreAllInputs(true);
    hg::Joystick::ignoreAllPresses(true);
}

void MenuGame::initLua(Lua::LuaContext& mLua)
{
    mLua.writeVariable(
        "u_log", [](string mLog) { lo("lua-menu") << mLog << "\n"; });

    mLua.writeVariable("u_execScript", [this, &mLua](string mName) {
        Utils::runLuaFile(mLua, levelData->packPath + "Scripts/" + mName);
    });

    mLua.writeVariable("u_getDifficultyMult", [] { return 1; });

    mLua.writeVariable("u_getSpeedMultDM", [] { return 1; });

    mLua.writeVariable("u_getDelayMultDM", [] { return 1; });

    mLua.writeVariable("u_getPlayerAngle", [] { return 0; });

    mLua.writeVariable("l_setRotationSpeed",
        [this](float mValue) { levelStatus.rotationSpeed = mValue; });

    mLua.writeVariable("l_setSides",
        [this](unsigned int mValue) { levelStatus.sides = mValue; });

    mLua.writeVariable(
        "l_getRotationSpeed", [this] { return levelStatus.rotationSpeed; });

    mLua.writeVariable("l_getSides", [this] { return levelStatus.sides; });

    mLua.writeVariable("l_set3dRequired",
        [this](bool mValue) { levelStatus._3DRequired = mValue; });

    mLua.writeVariable("s_setPulseInc",
        [this](float mValue) { styleData.pulseIncrement = mValue; });

    mLua.writeVariable("s_setPulseIncrement",
        [this](float mValue) { styleData.pulseIncrement = mValue; });

    mLua.writeVariable("s_setHueInc",
        [this](float mValue) { styleData.hueIncrement = mValue; });

    mLua.writeVariable("s_setHueIncrement",
        [this](float mValue) { styleData.hueIncrement = mValue; });

    mLua.writeVariable(
        "s_getHueInc", [this] { return styleData.hueIncrement; });

    mLua.writeVariable(
        "s_getHueIncrement", [this] { return styleData.hueIncrement; });

    // Unused functions
    for(const auto& un :
        {"l_setSpeedMult", "l_setPlayerSpeedMult", "l_setSpeedInc",
            "l_setSpeedMax", "l_getSpeedMax", "l_getDelayMin", "l_setDelayMin",
            "l_setDelayMax", "l_getDelayMax", "l_setRotationSpeedMax",
            "l_setRotationSpeedInc", "l_setDelayInc", "l_setFastSpin",
            "l_setSidesMin", "l_setSidesMax", "l_setIncTime", "l_setPulseMin",
            "l_setPulseMax", "l_setPulseSpeed", "l_setPulseSpeedR",
            "l_setPulseDelayMax", "l_setBeatPulseMax", "l_setBeatPulseDelayMax",
            "l_setBeatPulseInitialDelay", "l_setBeatPulseSpeedMult",
            "l_getBeatPulseInitialDelay", "l_getBeatPulseSpeedMult",
            "l_setWallSkewLeft", "l_setWallSkewRight", "l_setWallAngleLeft",
            "l_setWallAngleRight", "l_setRadiusMin", "l_setSwapEnabled",
            "l_setTutorialMode", "l_setIncEnabled", "l_get3dRequired",
            "l_enableRndSideChanges", "l_setDarkenUnevenBackgroundChunk",
            "l_getDarkenUnevenBackgroundChunk", "l_getSpeedMult",

            "l_getPlayerSpeedMult", "l_getDelayMult", "l_addTracked",
            "l_getRotation", "l_setRotation", "l_setDelayMult", "l_getOfficial",
            "l_overrideScore",

            "l_getSwapCooldownMult", "l_setSwapCooldownMult",

            "u_playSound", "u_isKeyPressed", "u_isMouseButtonPressed",
            "u_isFastSpinning", "u_setPlayerAngle", "u_forceIncrement",
            "u_kill", "u_eventKill", "u_haltTime", "u_timelineWait",
            "u_clearWalls", "u_setMusic", "u_setMusicSegment",
            "u_setMusicSeconds",

            "m_messageAdd", "m_messageAddImportant",
            "m_messageAddImportantSilent", "m_clearMessages",

            "t_eval", "t_wait", "t_waitS", "t_waitUntilS",

            "e_eval", "e_eventStopTime", "e_eventStopTimeS", "e_eventWait",
            "e_eventWaitS", "e_eventWaitUntilS",

            "w_wall", "w_wallAdj", "w_wallAcc", "w_wallHModSpeedData",
            "w_wallHModCurveData",

            "s_getCameraShake", "s_setCameraShake", "s_setStyle", "s_getHueMin",
            "s_setHueMin", "s_getHueMax", "s_setHueMax", "s_getPulseMin",
            "s_setPulseMin", "s_getPulseMax", "s_setPulseMax", "s_getPulseInc",
            "s_getPulseIncrement", "s_getHuePingPing", "s_setHuePingPong",
            "s_getMaxSwapTime", "s_setMaxSwapTime", "s_get3dDepth",
            "s_set3dDepth", "s_get3dSkew", "s_set3dSkew", "s_get3dSpacing",
            "s_set3dSpacing", "s_get3dDarkenMult", "s_set3dDarkenMult",
            "s_get3dAlphaMult", "s_set3dAlphaMult", "s_get3dAlphaFalloff",
            "s_set3dAlphaFalloff", "s_get3dPulseMax", "s_set3dPulseMax",
            "s_get3dPulseMin", "s_set3dPulseMin", "s_get3dPulseSpeed",
            "s_set3dPulseSpeed", "s_get3dPerspectiveMult",
            "s_set3dPerspectiveMult", "s_setCapColorMain",
            "s_setCapColorMainDarkened", "s_setCapColorByIndex",
            "s_setBGColorOffset", "s_getBGColorOffset", "s_setBGTileRadius",
            "s_getBGTileRadius", "s_setBGRotOff", "s_getBGRotOff",
            "steam_unlockAchievement",

            "cw_create", "cw_destroy", "cw_setVertexPos", "cw_setVertexColor",

            "cw_isOverlappingPlayer", "cw_clear"})
    {
        mLua.writeVariable(un, [] {});
    }
}

void MenuGame::setIndex(int mIdx)
{
    currentIndex = mIdx;

    if(currentIndex > ssvu::toInt(levelDataIds.size() - 1))
    {
        currentIndex = 0;
    }
    else if(currentIndex < 0)
    {
        currentIndex = ssvu::toInt(levelDataIds.size()) - 1;
    }

    levelData = &assets.getLevelData(levelDataIds.at(currentIndex));

    styleData = assets.getStyleData(levelData->packId, levelData->styleId);
    diffMults = levelData->difficultyMults;
    diffMultIdx = idxOf(diffMults, 1);

    Lua::LuaContext lua;
    initLua(lua);
    Utils::runLuaFile(lua, levelData->luaScriptPath);
    try
    {
        Utils::runLuaFunction<void>(lua, "onInit");
        Utils::runLuaFunction<void>(lua, "onLoad");
    }
    catch(std::runtime_error& mError)
    {
        std::cout << "[MenuGame::init] Runtime Lua error on menu "
                     "(onInit/onLoad) with level \""
                  << levelData->name << "\": \n"
                  << ssvu::toStr(mError.what()) << "\n"
                  << std::endl;

        if(!Config::getDebug())
        {
            assets.playSound("error.ogg");
        }
    }
}

void MenuGame::updateLeaderboard()
{
    if(assets.pIsLocal())
    {
        leaderboardString = "playing locally";
        return;
    }

    currentLeaderboard = Online::getCurrentLeaderboard();
    if(currentLeaderboard == "NULL")
    {
        leaderboardString = "...";
        return;
    }

    constexpr unsigned int leaderboardRecordCount{8};
    ssvuj::Obj root{getFromStr(currentLeaderboard)};
    if(getExtr<string>(root, "id") != levelData->id)
    {
        leaderboardString = "...";
        return;
    }

    auto currentPlayerScore = getExtr<string>(root, "ps");
    auto currentPlayerPosition = getExtr<string>(root, "pp");

    using RecordPair = pair<string, float>;
    vector<RecordPair> recordPairs;

    int playerPosition{-1};

    for(auto& record : ssvuj::getObj(root, "r"))
    {
        string name{toLower(getExtr<string>(record, 0))};
        float score{getExtr<float>(record, 1)};
        recordPairs.emplace_back(name, score);
    }

    bool foundPlayer{false};
    for(auto i(0u); i < recordPairs.size(); ++i)
    {
        if(recordPairs[i].first != assets.pGetName())
        {
            continue;
        }
        playerPosition = ssvu::toInt(i) + 1;
        foundPlayer = true;
        break;
    }

    string result;
    for(auto i(0u); i < recordPairs.size(); ++i)
    {
        if(currentPlayerScore != "NULL" && !currentPlayerScore.empty() &&
            !foundPlayer && i == leaderboardRecordCount - 1)
        {
            result.append("...(" + currentPlayerPosition + ") " +
                          assets.pGetName() + ": " + toStr(currentPlayerScore) +
                          "\n");
            break;
        }

        if(i <= leaderboardRecordCount)
        {
            if(playerPosition == -1 || i < leaderboardRecordCount)
            {
                auto& recordPair(recordPairs[i]);
                if(recordPair.first == assets.pGetName())
                {
                    result.append(" >> ");
                }
                result.append("(" + toStr(i + 1) + ") " + recordPair.first +
                              ": " + toStr(recordPair.second) + "\n");
            }
        }
        else
        {
            break;
        }
    }

    leaderboardString = result;
}

void MenuGame::updateFriends()
{
    if(state != States::SMain)
    {
        return;
    }

    if(assets.pIsLocal())
    {
        friendsString = "playing locally";
        return;
    }
    if(assets.pGetTrackedNames().empty())
    {
        friendsString = "you have no friends! :(\nadd them in the options menu";
        return;
    }

    const auto& fs(Online::getCurrentFriendScores());

    if(ssvuj::getObjSize(fs) == 0)
    {
        friendsString = "";
        for(const auto& n : assets.pGetTrackedNames())
        {
            friendsString.append("(?)" + n + "\n");
        }
        return;
    }

    using ScoreTuple = tuple<int, string, float>;
    vector<ScoreTuple> tuples;
    for(const auto& n : assets.pGetTrackedNames())
    {
        if(!ssvuj::hasObj(fs, n))
        {
            continue;
        }

        const auto& score(ssvuj::getExtr<float>(fs[n], 0));
        const auto& pos(ssvuj::getExtr<unsigned int>(fs[n], 1));

        if(pos == 0)
        {
            continue;
        }
        tuples.emplace_back(pos, n, score);
    }

    sort(tuples, [](const auto& mA, const auto& mB) {
        return std::get<0>(mA) < std::get<0>(mB);
    });
    friendsString.clear();
    for(const auto& t : tuples)
    {
        friendsString.append("(" + toStr(std::get<0>(t)) + ") " +
                             std::get<1>(t) + ": " + toStr(std::get<2>(t)) +
                             "\n");
    }
}

void MenuGame::refreshCamera()
{
    float fw{1024.f / Config::getWidth()};
    float fh{768.f / Config::getHeight()};
    float fmax{max(fw, fh)};
    w = Config::getWidth() * fmax;
    h = Config::getHeight() * fmax;
    overlayCamera.setView(View{FloatRect(0, 0, w, h)});
    titleBar.setOrigin(ssvs::zeroVec2f);
    titleBar.setScale({0.5f, 0.5f});
    titleBar.setPosition({20.f, 20.f});

    txtVersion.setString(Config::getVersionString());
    txtVersion.setFillColor(Color::White);
    txtVersion.setOrigin({getLocalRight(txtVersion), 0.f});
    txtVersion.setPosition(
        {getGlobalRight(titleBar) - 15.f, getGlobalTop(titleBar) + 15.f});

    creditsBar1.setOrigin({getLocalWidth(creditsBar1), 0.f});
    creditsBar1.setScale({0.373f, 0.373f});
    creditsBar1.setPosition({w - 20.f, 20.f});

    creditsBar2.setOrigin({getLocalWidth(creditsBar2), 0});
    creditsBar2.setScale({0.373f, 0.373f});
    creditsBar2.setPosition({w - 20.f, 17.f + getGlobalBottom(creditsBar1)});

    float scaleFactor{w / 1024.f};
    bottomBar.setOrigin({0, 56.f * 2.f});
    bottomBar.setScale({scaleFactor / 2.f, scaleFactor / 2.f});
    bottomBar.setPosition(sf::Vector2f(0, h));

    epilepsyWarning.setOrigin(getLocalCenter(epilepsyWarning));
    epilepsyWarning.setPosition({1024 / (2.f / scaleFactor), 768 / 2.f - 50});
    epilepsyWarning.setScale({0.36f, 0.36f});
}

void MenuGame::update(ssvu::FT mFT)
{
    steamManager.run_callbacks();
    discordManager.run_callbacks();

    hg::Joystick::update();

    if(hg::Joystick::leftRisingEdge())
    {
        leftAction();
    }
    else if(hg::Joystick::rightRisingEdge())
    {
        rightAction();
    }
    else if(hg::Joystick::upRisingEdge())
    {
        upAction();
    }
    else if(hg::Joystick::downRisingEdge())
    {
        downAction();
    }

    if(hg::Joystick::selectRisingEdge())
    {
        okAction();
    }
    else if(hg::Joystick::exitRisingEdge())
    {
        exitAction();
    }
    else if(hg::Joystick::createProfileRisingEdge())
    {
        createProfileAction();
    }
    else if(hg::Joystick::changePackRisingEdge())
    {
        selectPackAction();
    }
    else if(hg::Joystick::optionMenuRisingEdge())
    {
        openOptionsAction();
    }

    if(hg::Joystick::screenshotRisingEdge())
    {
        mustTakeScreenshot = true;
    }

    if(touchDelay > 0.f)
    {
        touchDelay -= mFT;
    }

    if(window.getFingerDownCount() == 1)
    {
        auto wThird = window.getWidth() / 3.f;
        auto wLT = window.getWidth() - wThird;
        auto hThird = window.getHeight() / 3.f;
        auto hLT = window.getHeight() - hThird;

        for(const auto& p : window.getFingerDownPositions())
        {
            if(p.y > hThird && p.y < hLT)
            {
                if(p.x > 0.f && p.x < wThird)
                {
                    leftAction();
                }
                else if(p.x < toNum<int>(window.getWidth()) && p.x > wLT)
                {
                    rightAction();
                }
                else if(p.x > wThird && p.x < wLT)
                {
                    okAction();
                }
            }
            else
            {
                if(p.y < hThird)
                {
                    upAction();
                }
                else if(p.y > hLT)
                {
                    downAction();
                }
            }
        }
    }

    overlayCamera.update(mFT);
    backgroundCamera.update(mFT);

    if(getCurrentMenu() != nullptr)
    {
        getCurrentMenu()->update();
    }

    currentCreditsId += mFT;
    creditsBar2.setTexture(assets.get<Texture>(
        ssvu::getByModIdx(creditsIds, ssvu::toInt(currentCreditsId / 100))));

    // If connection is lost, kick the player back into welcome screen
    if(!assets.pIsLocal() && Online::getConnectionStatus() != ocs::Connected)
    {
        state = States::MWlcm;
    }

    updateLeaderboard();
    updateFriends();

    if(exitTimer > 20)
    {
        window.stop();
    }

    if(isEnteringText())
    {
        unsigned int limit{state == States::ETEmail ? 40u : 18u};
        for(const auto& c : enteredChars)
        {
            if(enteredStr.size() < limit &&
                (ssvu::isAlphanumeric(c) || ssvu::isPunctuation(c)))
            {
                assets.playSound("beep.ogg");
                enteredStr.append(toStr(c));
            }
        }
    }
    else if(state == States::SLPSelect)
    {
        enteredStr =
            ssvu::getByModIdx(assets.getLocalProfileNames(), profileIdx);
    }
    else if(state == States::SMain)
    {
        styleData.update(mFT);
        backgroundCamera.turn(levelStatus.rotationSpeed * 10.f);

        if(!assets.pIsLocal())
        {
            float diffMult{ssvu::getByModIdx(diffMults, diffMultIdx)};
            Online::requestLeaderboardIfNeeded(levelData->id, diffMult);
        }
    }
    else if(state == States::SLogging)
    {
        if(Online::getLoginStatus() == ols::Logged)
        {
            state = Online::getNewUserReg() ? States::ETEmail : States::SMain;
        }
        else if(Online::getLoginStatus() == ols::Unlogged)
        {
            state = States::MWlcm;
        }
    }

    if(state == States::ETEmail && !Online::getNewUserReg())
        state = States::SMain;

    enteredChars.clear();
}

void MenuGame::draw()
{
    styleData.computeColors(levelStatus);
    window.clear(
        state != States::SMain ? Color::Black : styleData.getColors()[0]);

    backgroundCamera.apply();
    if(state == States::SMain)
    {
        styleData.drawBackground(window, ssvs::zeroVec2f, levelStatus);
    }

    overlayCamera.apply();
    if(state == States::SMain)
    {
        drawLevelSelection();
        render(bottomBar);
    }
    else if(isEnteringText())
    {
        drawEnteringText();
    }
    else if(state == States::SLPSelect)
    {
        drawProfileSelection();
    }
    else if(state == States::MOpts)
    {
        drawOptions();
    }
    else if(state == States::MWlcm)
    {
        drawWelcome();
    }
    else if(state == States::EpilepsyWarning)
    {
        render(epilepsyWarning);
        renderText("press any key to continue", txtProf, {20, 768 - 35});
        return;
    }

    render(titleBar);
    render(creditsBar1);
    render(creditsBar2);
    render(txtVersion);
    if(mustTakeScreenshot)
    {
        window.saveScreenshot("screenshot.png");
        mustTakeScreenshot = false;
    }

    if(hg::Config::getFullscreen())
    {
        window.setMouseCursorVisible(false);
    }
    else
    {
        window.setMouseCursorVisible(hg::Config::getMouseVisible());
    }

    if(!dialogBox.empty())
    {
        dialogBox.drawDialogBox();
    }
}

void MenuGame::drawLevelSelection()
{
    MusicData musicData{
        assets.getMusicData(levelData->packId, levelData->musicId)};

    const PackData& packData{assets.getPackData(levelData->packId)};
    const std::string& packName{packData.name};

    if(Config::getOnline())
    {
        std::string versionMessage{"connecting to server..."};
        float serverVersion{Online::getServerVersion()};

        if(serverVersion == -1)
        {
            versionMessage = "error connecting to server";
        }
        else if(serverVersion == Config::getVersion())
        {
            versionMessage = "you have the latest version";
        }
        else if(serverVersion < Config::getVersion())
        {
            versionMessage = "your version is newer (beta)";
        }
        else if(serverVersion > Config::getVersion())
        {
            versionMessage = "update available (" + toStr(serverVersion) + ")";
        }

        // TODO: restore online capabilities
        // renderText(versionMessage, txtProf, {20, 4}, 13);

        Text& profile = renderText("profile: " + assets.pGetName(), txtProf,
            sf::Vector2f{20.f, getGlobalBottom(titleBar) + 8}, 18);
        Text& pack =
            renderText("pack: " + packName + " (" + toStr(packIdx + 1) + "/" +
                           toStr(assets.getPackInfos().size()) + ")",
                txtProf, {20.f, getGlobalBottom(profile) - 7.f}, 18);

        std::string lbestStr;
        if(assets.pIsLocal())
        {
            SSVU_ASSERT(!diffMults.empty());
            lbestStr =
                "local best: " +
                toStr(assets.getLocalScore(getLocalValidator(
                    levelData->id, ssvu::getByModIdx(diffMults, diffMultIdx))));
        }
        else
        {
            lbestStr = Online::getLoginStatus() == ols::Logged
                           ? "logged in as: " + Online::getCurrentUsername()
                           : "logging in...";
        }

        Text& lbest = renderText(
            lbestStr, txtProf, {20.f, getGlobalBottom(pack) - 7.f}, 18);
        if(diffMults.size() > 1)
        {
            renderText("difficulty: " +
                           toStr(ssvu::getByModIdx(diffMults, diffMultIdx)),
                txtProf, {20.f, getGlobalBottom(lbest) - 7.f}, 18);
        }

        renderText(
            leaderboardString, txtProf, {20.f, getGlobalBottom(lbest)}, 15);

        // TODO: restore online capabilities
        // Text& smsg = renderText("server message: " +
        // Online::getServerMessage(),
        //    txtLAuth, {20.f, getGlobalTop(bottomBar) - 20.f}, 14);

        Text& smsg = renderText(
            "", txtLAuth, {20.f, getGlobalTop(bottomBar) - 20.f}, 14);

        // TODO: restore online capabilities
        /*
        txtFriends.setOrigin({getLocalWidth(txtFriends), 0.f});
        renderText("friends:\n" + friendsString, txtFriends,
            {w - 20.f, getGlobalBottom(titleBar) + 8}, 18);
        */

        // TODO: restore online capabilities
        /*
        if(!Config::isEligibleForScore())
        {
            renderText(
                "not eligible for scoring: " + Config::getUneligibilityReason(),
                txtProf, {20.f, getGlobalTop(smsg) - 20.f}, 11);
        }
        */

        if(!assets.pIsLocal() && Online::getLoginStatus() == ols::Logged)
        {
            const auto& us(Online::getUserStats());
            std::string userStats;
            userStats += "deaths: " + toStr(us.deaths) + "\n";
            userStats += "restarts: " + toStr(us.restarts) + "\n";
            userStats += "played: " + toStr(us.minutesSpentPlaying) + " min";
            renderText(userStats, txtLMus,
                {getGlobalRight(titleBar) + 10.f, getGlobalTop(titleBar)}, 13);
        }
    }
    else
    {
        renderText("online disabled", txtProf, {20, 0}, 13);
    }

    Text& lname = renderText(levelData->name, txtLName, {20.f, h / 2.f});
    Text& ldesc = renderText(
        levelData->description, txtLDesc, {20.f, getGlobalBottom(lname) - 5.f});
    Text& lauth = renderText("author: " + levelData->author, txtLAuth,
        {20.f, getGlobalBottom(ldesc) + 25.f});

    std::string musicString =
        "music: " + musicData.name + " by " + musicData.author;

    if(!musicData.album.empty())
    {
        musicString += " (" + musicData.album + ")";
    }

    renderText(musicString, txtLMus, {20.f, getGlobalBottom(lauth) - 5.f});
    renderText(
        "(" + toStr(currentIndex + 1) + "/" + toStr(levelDataIds.size()) + ")",
        txtLMus, {20.f, getGlobalTop(lname) - 30.f});

    std::string packNames{"Installed packs:\n"};
    std::string curPack;
    std::string longestPackName;

    for(const auto& n : assets.getPackInfos())
    {
        if(packData.id == n.id)
        {
            curPack = "  >>> ";
        }
        else
        {
            curPack = "      ";
        }

        const PackData& nPD{assets.getPackData(n.id)};
        curPack += nPD.name + " (by " + nPD.author + ") [v" +
                   std::to_string(nPD.version) + "]\n";
        packNames.append(curPack);

        // Width used to calculate origin should always be the longest pack name
        // + "  >>> " otherwise the list shifts around depending on the
        // currently selected item
        if(curPack.length() > longestPackName.length())
        {
            longestPackName = curPack;
        }
    }

    // calculate origin offset
    Utils::uppercasify(longestPackName);
    txtPacks.setString(longestPackName);
    float packsWidth = getGlobalWidth(txtPacks);

    // render packs list
    Utils::uppercasify(packNames);
    txtPacks.setString(packNames);
    txtPacks.setOrigin(packsWidth, getGlobalHeight(txtPacks));
    txtPacks.setPosition({w - 20.f, getGlobalTop(bottomBar) - 15.f});
    txtPacks.setFillColor(styleData.getTextColor());
    render(txtPacks);
}
void MenuGame::drawEnteringText()
{
    string title;
    switch(state)
    {
        case States::ETUser: title = "insert username"; break;
        case States::ETPass: title = "insert password"; break;
        case States::ETEmail: title = "insert email"; break;
        case States::ETFriend: title = "add friend"; break;
        case States::ETLPNew: title = "create local profile"; break;
        default: throw;
    }

    renderText(title, txtProf, {20, 768 - 395});
    renderText("insert text", txtProf, {20, 768 - 375});
    renderText("press enter when done", txtProf, {20, 768 - 335});
    renderText("keep esc pressed to exit", txtProf, {20, 768 - 315});
    renderText(
        state == States::ETPass ? string(enteredStr.size(), '*') : enteredStr,
        txtLName, {20, 768 - 245 - 40}, (state == States::ETEmail) ? 32 : 65);
}
void MenuGame::drawProfileSelection()
{
    if(!assets.pIsLocal())
    {
        throw;
    }
    renderText("local profile selection", txtProf, {20, 768 - 395});
    renderText("press left/right to browse profiles", txtProf, {20, 768 - 375});
    renderText("press enter to select profile", txtProf, {20, 768 - 355});
    renderText("press f1 to create a new profile", txtProf, {20, 768 - 335});
    renderText(enteredStr, txtLName, {20, 768 - 245 - 40});
}

void MenuGame::drawMenu(const Menu& mMenu)
{
    renderText(mMenu.getCategory().getName(), txtLDesc,
        {20.f, getGlobalBottom(titleBar)});

    float currentX{0.f};
    float currentY{0.f};
    const auto& currentItems(mMenu.getItems());
    for(int i{0}; i < ssvu::toInt(currentItems.size()); ++i)
    {
        currentY += 19;
        if(i != 0 && i % 21 == 0)
        {
            currentY = 0;
            currentX += 280;
        }
        string name;
        string itemName{currentItems[i]->getName()};
        if(i == mMenu.getIdx())
        {
            name.append(">> ");
        }
        name.append(itemName);

        int extraSpacing{0};
        if(itemName == "back")
        {
            extraSpacing = 20;
        }
        renderText(name, txtProf,
            {20.f + currentX,
                getGlobalBottom(titleBar) + 20.f + currentY + extraSpacing},
            currentItems[i]->isEnabled() ? Color::White
                                         : Color{155, 155, 155, 255});
    }
}

void MenuGame::drawOptions()
{
    drawMenu(optionsMenu);

    if(Config::getOfficial())
    {
        renderText("official mode on - some options cannot be changed", txtProf,
            {20, h - 30.f});
    }
    else
    {
        renderText("official mode off - not eligible for scoring", txtProf,
            {20, h - 30.f});
    }

    if(assets.pIsLocal())
    {
        renderText("local mode on - some options cannot be changed", txtProf,
            {20, h - 60.f});
    }
}

void MenuGame::drawWelcome()
{
    drawMenu(welcomeMenu);

    renderText(Online::getLoginStatus() == ols::Logged
                   ? "logged in as: " + Online::getCurrentUsername()
                   : "not logged in",
        txtProf, {20, h - 50.f});

    string connStatus;
    switch(Online::getConnectionStatus())
    {
        case ocs::Disconnected: connStatus = "not connected to server"; break;
        case ocs::Connecting: connStatus = "connecting to server..."; break;
        case ocs::Connected: connStatus = "connected to server"; break;
    }
    renderText(connStatus, txtProf, {20, h - 30.f});
}

} // namespace hg

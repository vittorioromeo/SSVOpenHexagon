// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/MenuGame.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Core/Discord.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Core/BindControl.hpp"

#include <SSVStart/Input/Input.hpp>
#include <SSVStart/Utils/Vector2.hpp>

#include <SSVMenuSystem/SSVMenuSystem.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <utility>
#include <array>
#include <string_view>

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

//*****************************************************
//
// INITIALIZATION
//
//*****************************************************

inline constexpr float maxOffset = 100.f;
inline constexpr std::string_view favoritePath = "Assets/favoriteLevels.json";

MenuGame::MenuGame(Steam::steam_manager& mSteamManager,
    Discord::discord_manager& mDiscordManager, HGAssets& mAssets,
    HexagonGame& mHexagonGame, GameWindow& mGameWindow)
    : steamManager(mSteamManager), discordManager(mDiscordManager),
      assets(mAssets), hexagonGame(mHexagonGame), window(mGameWindow),
      dialogBox(mAssets, mGameWindow, styleData),
      loadInfo(mAssets.getLoadResults())
{
    if(Config::getFirstTimePlaying())
    {
        showFirstTimeTips = true;
        Config::setFirstTimePlaying(false);
    }

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

    game.onEvent(
        Event::EventType::MouseWheelMoved) += [this](const Event& mEvent) {
        // Disable scroll while assigning a bind
        if(state == States::MOpts)
        {
            const auto* const bc{
                dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem())};
            if(bc != nullptr && bc->isWaitingForBind())
            {
                return;
            }
        }

        if(state == States::LevelSelection && focusHeld)
        {
            changePackQuick(mEvent.mouseWheel.delta > 0 ? -1 : 1);
            return;
        }

        wheelProgress += mEvent.mouseWheel.delta;
        if(wheelProgress > 1.f)
        {
            wheelProgress = 0.f;
            upAction();
        }
        else if(wheelProgress < -1.f)
        {
            wheelProgress = 0.f;
            downAction();
        }
    };

    // To close the load results with any key
    setIgnoreAllInputs(1);

    const auto checkCloseBootScreens = [this] {
        if(!(--ignoreInputs))
        {
            if(state == States::LoadingScreen)
            {
                changeStateTo(States::EpilepsyWarning);
                setIgnoreAllInputs(1);
                scrollbarOffset = 0;
            }
            else
            {
                mainMenu.getItems()[0]->getOffset() = maxOffset;
                mainMenu.getCategory().getOffset() =
                    fourByThree ? 280.f : 400.f;

                // TODO: remove when welcome screen is implemented
                playLocally();
                setIgnoreAllInputs(0);
            }
            assets.playSound("select.ogg");
        }
    };

    const auto checkCloseDialogBox = [this] {
        if(!ignoreInputs)
        {
            dialogBox.clearDialogBox();
            setIgnoreAllInputs(0);
        }
    };

    game.onEvent(Event::EventType::KeyReleased) += [this, checkCloseBootScreens,
                                                       checkCloseDialogBox](
                                                       const Event& mEvent) {
        // don't do anything if inputs are being processed as usual
        if(!ignoreInputs)
        {
            return;
        }

        // Scenario one: epilepsy warning is being drawn and user
        // must close it with any key press
        if(state == States::EpilepsyWarning || state == States::LoadingScreen)
        {
            checkCloseBootScreens();
            return;
        }

        // Scenario two: actions are blocked cause a dialog box is open
        if(dialogBoxDelay > 0.f)
        {
            return;
        }

        KKey key{mEvent.key.code};
        if(!dialogBox.empty())
        {
            if(dialogBox.getKeyToClose() == KKey::Unknown ||
                key == dialogBox.getKeyToClose())
            {
                --ignoreInputs;
            }
            checkCloseDialogBox();
            return;
        }

        // Scenario three: actions are blocked cause we are using a
        // BindControl menu item
        if(getCurrentMenu() != nullptr && key == KKey::Escape)
        {
            getCurrentMenu()->getItem().exec(); // turn off bind inputting
            setIgnoreAllInputs(0);
            assets.playSound("beep.ogg");
            return;
        }

        if(!(--ignoreInputs))
        {
            if(getCurrentMenu() == nullptr || state != States::MOpts)
            {
                setIgnoreAllInputs(0);
                return;
            }

            auto* const bc{dynamic_cast<KeyboardBindControl*>(
                &getCurrentMenu()->getItem())};

            // don't try assigning a keyboard key to a controller bind
            if(bc == nullptr)
            {
                assets.playSound("error.ogg");
                ignoreInputs = 1;
                return;
            }

            // If user tries to bind a key that is already hardcoded ignore
            // the input and notify it of what has happened.
            if(!bc->newKeyboardBind(key))
            {
                assets.playSound("error.ogg");
                setIgnoreAllInputs(1);
                dialogBox.create(
                    "THE KEY YOU ARE TRYING TO ASSIGN TO THIS ACTION\n"
                    "IS ALREADY BOUND TO IT BY DEFAULT,\n"
                    "YOUR LAST INPUT HAS BEEN IGNORED\n\n"
                    "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n",
                    26, 10.f, DBoxDraw::center);
                return;
            }

            assets.playSound("select.ogg");
            setIgnoreAllInputs(0);
            touchDelay = 10.f;
        }
    };

    game.onEvent(Event::EventType::MouseButtonReleased) +=
        [this, checkCloseBootScreens, checkCloseDialogBox](
            const Event& mEvent) {
            if(!ignoreInputs)
            {
                return;
            }

            if(state == States::EpilepsyWarning ||
                state == States::LoadingScreen)
            {
                checkCloseBootScreens();
                return;
            }

            if(dialogBoxDelay > 0.f)
            {
                return;
            }

            if(!dialogBox.empty())
            {
                if(dialogBox.getKeyToClose() != KKey::Unknown)
                {
                    return;
                }
                --ignoreInputs;
                checkCloseDialogBox();
                return;
            }

            if(!(--ignoreInputs))
            {
                if(getCurrentMenu() == nullptr || state != States::MOpts)
                {
                    setIgnoreAllInputs(0);
                    return;
                }

                auto* const bc{dynamic_cast<KeyboardBindControl*>(
                    &getCurrentMenu()->getItem())};

                // don't try assigning a keyboard key to a controller bind
                if(bc == nullptr)
                {
                    assets.playSound("error.ogg");
                    ignoreInputs = 1;
                    return;
                }

                bc->newKeyboardBind(mEvent.mouseButton.button);
                assets.playSound("select.ogg");
                setIgnoreAllInputs(0);
                touchDelay = 10.f;
            }
        };

    game.onEvent(Event::EventType::JoystickButtonReleased) +=
        [this, checkCloseBootScreens, checkCloseDialogBox](
            const Event& mEvent) {
            if(!ignoreInputs)
            {
                return;
            }

            if(state == States::EpilepsyWarning ||
                state == States::LoadingScreen)
            {
                checkCloseBootScreens();
                return;
            }

            if(dialogBoxDelay > 0.f)
            {
                return;
            }

            if(!dialogBox.empty())
            {
                if(dialogBox.getKeyToClose() != KKey::Unknown)
                {
                    return;
                }
                --ignoreInputs;
                checkCloseDialogBox();
                return;
            }

            if(!(--ignoreInputs))
            {
                if(getCurrentMenu() == nullptr || state != States::MOpts)
                {
                    setIgnoreAllInputs(0);
                    return;
                }

                auto* const bc{dynamic_cast<JoystickBindControl*>(
                    &getCurrentMenu()->getItem())};

                // don't try assigning a controller button to a keyboard bind
                if(bc == nullptr)
                {
                    assets.playSound("error.ogg");
                    ignoreInputs = 1;
                    return;
                }

                bc->newJoystickBind(mEvent.joystickButton.button);
                setIgnoreAllInputs(0);
                assets.playSound("select.ogg");
                touchDelay = 10.f;
            }
        };

    window.onRecreation += [this] { refreshCamera(); };

    initMenus();
    initInput();
    initLua();

    //--------------------------------
    // Main menu background

    const auto [randomPack, randomLevel] = pickRandomMainMenuBackgroundStyle();
    lvlDrawer->levelDataIds =
        assets.getLevelIdsByPack(assets.getPackInfos().at(randomPack).id);
    setIndex(randomLevel);

    // Setup for the loading menu
    static constexpr std::array<std::array<std::string_view, 2>, 4> tips{
        {{"HOLDING SHIFT WHILE CHANGING PACK", "SKIPS THE SWITCH ANIMATION"},
            {"REMEMBER TO TAKE BREAKS", "OPEN HEXAGON IS AN INTENSE GAME"},
            {"EXPERIMENT USING SWAP", "IT MAY SAVE YOUR LIFE"},
            {"IF A LEVEL IS TOO CHALLENGING",
                "PRACTICE IT AT A LOWER DIFFICULTY"}}};
    randomTip = tips[ssvu::getRndI(0, tips.size())];

    // Set size of the level offsets vector to the minimum required
    unsigned int maxSize{0}, packSize;
    for(size_t i{0}; i < getPackInfosSize(); ++i)
    {
        packSize =
            assets.getLevelIdsByPack(assets.getPackInfos().at(i).id).size();
        if(packSize > maxSize)
        {
            maxSize = packSize;
        }
    }
    lvlSlct.lvlOffsets.resize(maxSize);

    //--------------------------------
    // Favorite levels

    const std::string fp{std::string(favoritePath)};
    if(!ssvufs::Path{fp}.exists<ssvufs::Type::File>())
    {
        return;
    }

    // Load the stored favorite levels IDs from file.
    favSlct.levelDataIds =
        getExtr<std::vector<std::string>>(getFromFile(fp), "ids");
    // Verify the levels with corresponding IDs actually exist.
    // If they don't remove them.
    auto it{favSlct.levelDataIds.begin()};
    while(it != favSlct.levelDataIds.end())
    {
        if(!assets.checkLevelIDPurity(*it))
        {
            favSlct.levelDataIds.erase(it);
            continue;
        }
        // if the level exists mark the corresponding LevelData structs as
        // a favorite. This is needed to show wherever the current level can
        // be added or removed from the favorites.
        assets.setLevelFavoriteFlag(*it++, true);
    }
    // Sort levels based on the name.
    std::sort(favSlct.levelDataIds.begin(), favSlct.levelDataIds.end(),
        [this](const std::string& a, const std::string& b) -> bool {
            return assets.getLevelData(a).name < assets.getLevelData(b).name;
        });
    // Set size of the level offsets vector to the required amount.
    favSlct.lvlOffsets.resize(static_cast<int>(favSlct.levelDataIds.size()));
}

void MenuGame::init(bool error)
{
    steamManager.set_rich_presence_in_menu();
    steamManager.update_hardcoded_achievements();

    discordManager.set_rich_presence_in_menu();

    assets.stopMusics();
    assets.stopSounds();

    if(error)
    {
        assets.playSound("error.ogg");
    }
    else
    {
        assets.playSound("select.ogg");
    }

    // Online::setForceLeaderboardRefresh(true);
}

void MenuGame::init(
    bool error, const std::string& pack, const std::string& level)
{
    init(error);
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

void MenuGame::changeStateTo(const States mState)
{
    const States prevState = state;
    state = mState;

    if(prevState == state)
    {
        // Not a state transition.
        return;
    }

    if(!showFirstTimeTips)
    {
        // Not the first time playing.
        return;
    }

    const auto mustShowTip = [&](const States s, bool& flag) {
        return state == s && std::exchange(flag, false);
    };

    const auto showTip = [&](const char* str) {
        assets.playSound("beep.ogg");

        dialogBox.create(str, 26, 10.f, DBoxDraw::center);
        setIgnoreAllInputs(1);

        // Prevent dialog box from being closed immediately:
        dialogBoxDelay = 64.f;
    };

    if(mustShowTip(States::SMain, mustShowFTTMainMenu))
    {
        showTip(
            "WELCOME TO OPEN HEXAGON!\n\n"
            "YOU CAN NAVIGATE THE MAIN MENU WITH THE UP/DOWN ARROW KEYS\n"
            "OR THE DPAD/THUMBSTICK ON YOUR CONTROLLER\n\n"
            "REMEMBER TO CHECK OUT THE OPTIONS MENU\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
    }
    else if(mustShowTip(States::LevelSelection, mustShowFTTLevelSelect))
    {
        showTip(
            "THIS IS WHERE YOU CAN PICK A LEVEL TO PLAY\n\n"
            "LEVELS ARE ORGANIZED IN 'LEVEL PACKS'\n"
            "TO BROWSE LEVELS AND LEVEL PACKS, GO UP/DOWN\n"
            "HOLD SHIFT (FOCUS) TO QUICKLY JUMP BETWEEN PACKS\n"
            "TO CHANGE THE DIFFICULTY OF A LEVEL, GO LEFT/RIGHT\n\n"
            "AS A FIRST TIMER, PLAY THE 'CUBE' LEVELS IN ORDER\n"
            "THEN YOU CAN GET NEW LEVELS ON THE STEAM WORKSHOP\n\n"
            "HAVE FUN!\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
    }
    else if(mustShowTip(States::LevelSelection, mustShowFTTDeathTips))
    {
        showTip(
            "IF YOU FEEL LIKE THE GAME IS TOO HARD, TRY THESE TIPS\n\n"
            "- DECREMENT THE DIFFICULTY BY PRESSING LEFT\n"
            "- PLAY AN EASIER LEVEL TO BUILD UP MUSCLE MEMORY\n"
            "- TURN ON 'INVINCIBILITY' IN 'GAMEPLAY' OPTIONS TO PRACTICE\n\n"
            "REMEMBER THAT PRACTICE IS THE SECRET TO SUCCESS!\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n");
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

    game.addInput( // hardcoded
        {{k::Up}}, [this](ssvu::FT /*unused*/) { upAction(); }, t::Once);

    game.addInput(
        Config::getTriggerUp(), [this](ssvu::FT /*unused*/) { upAction(); },
        t::Once, Tid::Up);

    game.addInput( // hardcoded
        {{k::Down}}, [this](ssvu::FT /*unused*/) { downAction(); }, t::Once);

    game.addInput(
        Config::getTriggerDown(), [this](ssvu::FT /*unused*/) { downAction(); },
        t::Once, Tid::Down);

    game.addInput(
        Config::getTriggerNextPack(),
        [this](ssvu::FT /*unused*/) { changePackAction(1); }, t::Once,
        Tid::NextPack);

    game.addInput(
        Config::getTriggerPreviousPack(),
        [this](ssvu::FT /*unused*/) { changePackAction(-1); }, t::Once,
        Tid::PreviousPack);

    add2StateInput(game, Config::getTriggerFocus(), focusHeld, Tid::Focus);

    game.addInput( // hardcoded
        {{k::Return}}, [this](ssvu::FT /*unused*/) { okAction(); }, t::Once);

    game.addInput( // hardcoded
        {{k::Escape}},
        [this](ssvs::FT mFT) {
            if(state != States::MOpts)
            {
                exitTimer += mFT;
            }
        },
        [this](ssvu::FT /*unused*/) { exitTimer = 0; }, t::Always);

    game.addInput( // hardcoded
        {{k::Escape}}, [this](ssvu::FT /*unused*/) { exitAction(); }, t::Once);

    game.addInput(
        Config::getTriggerExit(),
        [this](ssvu::FT /*unused*/) {
            if(isEnteringText())
            {
                return;
            }
            exitAction();
        },
        t::Once, Tid::Exit); // editable

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

    game.addInput( // hardcoded
        {{k::BackSpace}}, [this](ssvu::FT /*unused*/) { eraseAction(); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F1}}, [this](ssvu::FT /*unused*/) { changeLevelFavoriteFlag(); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F2}},
        [this](ssvu::FT /*unused*/) { switchToFromFavoriteLevels(); }, t::Once);

    game.addInput( // hardcoded
        {{k::F3}}, [this](ssvu::FT /*unused*/) { reloadAssets(false); },
        t::Once);

    game.addInput( // hardcoded
        {{k::F4}}, [this](ssvu::FT /*unused*/) { reloadAssets(true); },
        t::Once);
}

void MenuGame::initLua()
{
    lua.writeVariable(
        "u_log", [](string mLog) { lo("lua-menu") << mLog << "\n"; });

    lua.writeVariable("u_execScript", [this](string mName) {
        Utils::runLuaFile(lua, levelData->packPath + "Scripts/" + mName);
    });

    lua.writeVariable("u_getDifficultyMult", [] { return 1; });

    lua.writeVariable("u_getSpeedMultDM", [] { return 1; });

    lua.writeVariable("u_getDelayMultDM", [] { return 1; });

    lua.writeVariable("u_getPlayerAngle", [] { return 0; });

    lua.writeVariable("l_setRotationSpeed",
        [this](float mValue) { levelStatus.rotationSpeed = mValue; });

    lua.writeVariable("l_setSides",
        [this](unsigned int mValue) { levelStatus.sides = mValue; });

    lua.writeVariable(
        "l_getRotationSpeed", [this] { return levelStatus.rotationSpeed; });

    lua.writeVariable("l_getSides", [this] { return levelStatus.sides; });

    lua.writeVariable("l_set3dRequired",
        [this](bool mValue) { levelStatus._3DRequired = mValue; });

    lua.writeVariable("s_setPulseInc",
        [this](float mValue) { styleData.pulseIncrement = mValue; });

    lua.writeVariable("s_setPulseIncrement",
        [this](float mValue) { styleData.pulseIncrement = mValue; });

    lua.writeVariable("s_setHueInc",
        [this](float mValue) { styleData.hueIncrement = mValue; });

    lua.writeVariable("s_setHueIncrement",
        [this](float mValue) { styleData.hueIncrement = mValue; });

    lua.writeVariable("l_getPulseMin", [this] { return levelStatus.pulseMin; });
    lua.writeVariable("l_getPulseMax", [this] { return levelStatus.pulseMax; });
    lua.writeVariable(
        "l_getPulseSpeed", [this] { return levelStatus.pulseSpeed; });
    lua.writeVariable(
        "l_getPulseSpeedR", [this] { return levelStatus.pulseSpeedR; });

    lua.writeVariable(
        "u_getVersionMajor", [] { return Config::getVersion().major; });
    lua.writeVariable(
        "u_getVersionMinor", [] { return Config::getVersion().minor; });
    lua.writeVariable(
        "u_getVersionMicro", [] { return Config::getVersion().micro; });
    lua.writeVariable(
        "u_getVersionString", [] { return Config::getVersionString(); });

    lua.writeVariable("l_setRotationSpeed",
        [this](float mValue) { levelStatus.rotationSpeed = mValue; });

    lua.writeVariable("s_getHueInc", [this] { return styleData.hueIncrement; });

    lua.writeVariable(
        "s_getHueIncrement", [this] { return styleData.hueIncrement; });

    // Unused functions
    for(const auto& un : {"u_isKeyPressed", "u_isMouseButtonPressed",
            "u_isFastSpinning", "u_setPlayerAngle", "u_forceIncrement",
            "u_haltTime", "u_timelineWait", "u_clearWalls",

            "a_setMusic", "a_setMusicSegment", "a_setMusicSeconds",
            "a_playSound", "a_playPackSound", "a_syncMusicToDM",
            "a_setMusicPitch", "a_overrideBeepSound",
            "a_overrideIncrementSound", "a_overrideSwapSound",
            "a_overrideDeathSound",

            "t_eval", "t_kill", "t_clear", "t_wait", "t_waitS", "t_waitUntilS",

            "e_eval", "e_kill", "e_stopTime", "e_stopTimeS", "e_wait",
            "e_waitS", "e_waitUntilS", "e_messageAdd", "e_messageAddImportant",
            "e_messageAddImportantSilent", "e_clearMessages",

            "l_setSpeedMult", "l_setPlayerSpeedMult", "l_setSpeedInc",
            "l_setSpeedMax", "l_getSpeedMax", "l_getDelayMin", "l_setDelayMin",
            "l_setDelayMax", "l_getDelayMax", "l_setRotationSpeedMax",
            "l_setRotationSpeedInc", "l_setDelayInc", "l_setFastSpin",
            "l_setSidesMin", "l_setSidesMax", "l_setIncTime", "l_setPulseMin",
            "l_setPulseMax", "l_setPulseSpeed", "l_setPulseSpeedR",
            "l_setPulseDelayMax", "l_setBeatPulseMax", "l_setBeatPulseDelayMax",
            "l_setBeatPulseInitialDelay", "l_setBeatPulseSpeedMult",
            "l_getBeatPulseInitialDelay", "l_getBeatPulseSpeedMult",
            "l_setWallSkewLeft", "l_setWallSkewRight", "l_setWallAngleLeft",
            "l_setWallAngleRight", "l_getWallSpawnDistance",
            "l_setWallSpawnDistance", "l_setRadiusMin", "l_setSwapEnabled",
            "l_setTutorialMode", "l_setIncEnabled", "l_get3dRequired",
            "l_enableRndSideChanges", "l_setDarkenUnevenBackgroundChunk",
            "l_getDarkenUnevenBackgroundChunk", "l_getSpeedMult",

            "l_getPlayerSpeedMult", "l_getDelayMult", "l_addTracked",
            "l_getRotation", "l_setRotation", "l_setDelayMult", "l_getOfficial",
            "l_overrideScore",

            "l_getSwapCooldownMult", "l_setSwapCooldownMult",

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

            "w_wall", "w_wallAdj", "w_wallAcc", "w_wallHModSpeedData",
            "w_wallHModCurveData",

            "cw_create", "cw_destroy", "cw_setVertexPos", "cw_setVertexColor",
            "cw_setCollision", "cw_getVertexPos",
            "cw_isOverlappingPlayer", "cw_clear", "cw_getCollision",

            "steam_unlockAchievement",

            "u_kill", "u_eventKill", "u_playSound", "u_playPackSound",
            "e_eventStopTime", "e_eventStopTimeS", "e_eventWait",
            "e_eventWaitS", "e_eventWaitUntilS", "m_messageAdd",
            "m_messageAddImportant", "m_messageAddImportantSilent",
            "m_clearMessages"})
    {
        lua.writeVariable(un, [] {});
    }
}

void MenuGame::initMenus()
{
    namespace i = ssvms::Items;

    auto whenLocal = [this] { return assets.pIsLocal(); };
    auto whenNotOfficial = [] { return !Config::getOfficial(); };
    auto whenUnlogged = [] { return true; };
    auto whenSoundEnabled = [] { return !Config::getNoSound(); };
    auto whenMusicEnabled = [] { return !Config::getNoMusic(); };
    auto whenTimerIsStatic = [] { return Config::getTimerStatic(); };
    auto whenTimerIsDynamic = [] { return !Config::getTimerStatic(); };


    // Welcome menu
    auto& wlcm(welcomeMenu.createCategory("welcome"));
    wlcm.create<i::Single>("play locally", [this] { playLocally(); }) |
        whenUnlogged;
    wlcm.create<i::Single>("exit game", [this] { window.stop(); });

    //--------------------------------
    // OPTIONS MENU
    //--------------------------------

    auto& options(optionsMenu.createCategory("options"));
    auto& friends(optionsMenu.createCategory("friends"));
    auto& play(optionsMenu.createCategory("gameplay"));
    auto& controls(optionsMenu.createCategory("controls"));
    auto& keyboard(optionsMenu.createCategory("keyboard"));
    auto& joystick(optionsMenu.createCategory("joystick"));
    auto& resolution(optionsMenu.createCategory("resolution"));
    auto& gfx(optionsMenu.createCategory("graphics"));
    auto& sfx(optionsMenu.createCategory("audio"));

    options.create<i::Goto>("GAMEPLAY", play);
    options.create<i::Goto>("CONTROLS", controls);
    options.create<i::Goto>("RESOLUTION", resolution);
    options.create<i::Goto>("GRAPHICS", gfx);
    options.create<i::Goto>("AUDIO", sfx);
    options.create<i::Single>("RESET CONFIG", [this] {
        Config::resetConfigToDefaults();
        refreshBinds();
    });

    // TODO:
    // options.create<i::Single>("login screen", [this] {
    // changeStateTo(States::MWlcm);
    // });
    // options.create<i::Toggle>("online", &Config::getOnline,
    // &Config::setOnline);

    //--------------------------------
    // Gameplay

    play.create<i::Toggle>(
        "autorestart", &Config::getAutoRestart, &Config::setAutoRestart);
    play.create<i::Toggle>("rotate to start", &Config::getRotateToStart,
        &Config::setRotateToStart);
    play.create<i::Toggle>(
        "OFFICIAL MODE", &Config::getOfficial, &Config::setOfficial);
    play.create<i::Toggle>("debug mode", &Config::getDebug, &Config::setDebug) |
        whenNotOfficial;
    play.create<i::Toggle>(
        "invincible", &Config::getInvincible, &Config::setInvincible) |
        whenNotOfficial;
    play.create<i::Slider>("timescale", &Config::getTimescale,
        &Config::setTimescale, 0.1f, 2.f, 0.05f) |
        whenNotOfficial;
    play.create<i::GoBack>("back");

    //--------------------------------
    // Controls

    controls.create<i::Goto>("keyboard", keyboard);
    controls.create<i::Goto>("joystick", joystick);
    controls.create<i::Slider>("joystick deadzone",
        &Config::getJoystickDeadzone, &Config::setJoystickDeadzone, 0.f, 100.f,
        1.f);
    controls.create<i::Single>("reset binds", [this] {
        Config::resetBindsToDefaults();
        refreshBinds();
    });
    controls.create<i::Single>("hardcoded keys reference", [this] {
        dialogBox.create(
            "UP ARROW - UP\n"
            "DOWN ARROW - DOWN\n"
            "RETURN - ENTER\n"
            "BACKSPACE - REMOVE BIND\n"
            "F1 - ADD LEVEL TO FAVORITES\n"
            "F2 - SWITCH TO/FROM FAVORITE LEVELS\n"
            "F3 - RELOAD LEVEL ASSETS (DEBUG MODE ONLY)\n"
            "F4 - RELOAD PACK ASSETS (DEBUG MODE ONLY)\n\n"
            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n",
            26, 10.f, DBoxDraw::center);
        setIgnoreAllInputs(2);
    });
    controls.create<i::GoBack>("back");

    // Keyboard binds
    auto callBack = [this](const Trigger& trig, const int bindID) {
        game.refreshTrigger(trig, bindID);
        hexagonGame.refreshTrigger(trig, bindID);
    };

    keyboard.create<KeyboardBindControl>("rotate ccw",
        &Config::getTriggerRotateCCW, &Config::addBindTriggerRotateCCW,
        &Config::clearBindTriggerRotateCCW, callBack, Tid::RotateCCW);
    keyboard.create<KeyboardBindControl>("rotate cw",
        &Config::getTriggerRotateCW, &Config::addBindTriggerRotateCW,
        &Config::clearBindTriggerRotateCW, callBack, Tid::RotateCW);
    keyboard.create<KeyboardBindControl>("focus", &Config::getTriggerFocus,
        &Config::addBindTriggerFocus, &Config::clearBindTriggerFocus, callBack,
        Tid::Focus);
    keyboard.create<KeyboardBindControl>("exit", &Config::getTriggerExit,
        &Config::addBindTriggerExit, &Config::clearBindTriggerExit, callBack,
        Tid::Exit, KKey::Escape);
    keyboard.create<KeyboardBindControl>("force restart",
        &Config::getTriggerForceRestart, &Config::addBindTriggerForceRestart,
        &Config::clearBindTriggerForceRestart, callBack, Tid::ForceRestart);
    keyboard.create<KeyboardBindControl>("restart", &Config::getTriggerRestart,
        &Config::addBindTriggerRestart, &Config::clearBindTriggerRestart,
        callBack, Tid::Restart);
    keyboard.create<KeyboardBindControl>("replay", &Config::getTriggerReplay,
        &Config::addBindTriggerReplay, &Config::clearBindTriggerReplay,
        callBack, Tid::Replay);
    keyboard.create<KeyboardBindControl>("screenshot",
        &Config::getTriggerScreenshot, &Config::addBindTriggerScreenshot,
        &Config::clearBindTriggerScreenshot, callBack, Tid::Screenshot);
    keyboard.create<KeyboardBindControl>("swap", &Config::getTriggerSwap,
        &Config::addBindTriggerSwap, &Config::clearBindTriggerSwap, callBack,
        Tid::Swap);
    keyboard.create<KeyboardBindControl>("up", &Config::getTriggerUp,
        &Config::addBindTriggerUp, &Config::clearBindTriggerUp, callBack,
        Tid::Up, KKey::Up);
    keyboard.create<KeyboardBindControl>("down", &Config::getTriggerDown,
        &Config::addBindTriggerDown, &Config::clearBindTriggerDown, callBack,
        Tid::Down, KKey::Down);
    keyboard.create<KeyboardBindControl>("next pack",
        &Config::getTriggerNextPack, &Config::addBindTriggerNextPack,
        &Config::clearBindTriggerNextPack, callBack, Tid::NextPack);
    keyboard.create<KeyboardBindControl>("previous pack",
        &Config::getTriggerPreviousPack, &Config::addBindTriggerPreviousPack,
        &Config::clearBindTriggerPreviousPack, callBack, Tid::PreviousPack);
    keyboard.create<i::GoBack>("back");

    // Joystick binds
    using Jid = hg::Joystick::Jid;

    auto JoystickCallBack = [](const unsigned int button, const int buttonID) {
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
    joystick.create<JoystickBindControl>("next pack",
        &Config::getJoystickNextPack, &Config::reassignToJoystickNextPack,
        JoystickCallBack, Jid::NextPack);
    joystick.create<JoystickBindControl>("previous pack",
        &Config::getJoystickPreviousPack,
        &Config::reassignToJoystickPreviousPack, JoystickCallBack,
        Jid::PreviousPack);
    joystick.create<i::GoBack>("back");

    //--------------------------------
    // Resolution

    resolution.create<i::Single>("automatically set resolution",
        [this] { Config::setCurrentResolutionAuto(window); });

    auto& sixByNine(optionsMenu.createCategory("16x9 resolutions"));
    auto& fourByThree(optionsMenu.createCategory("4x3 resolutions"));
    auto& sixByTen(optionsMenu.createCategory("16x10 resolutions"));

    resolution.create<i::Goto>("16x9 resolutions", sixByNine);
    resolution.create<i::Goto>("4x3 resolutions", fourByThree);
    resolution.create<i::Goto>("16x10 - uncommon resolutions", sixByTen);

    int ratio;
    for(const auto& vm : VideoMode::getFullscreenModes())
    {
        if(vm.bitsPerPixel == 32)
        {
            ratio = 10.f * vm.width / vm.height;

            switch(ratio)
            {
                case 17: // 16:9
                    sixByNine.create<i::Single>(
                        toStr(vm.width) + "x" + toStr(vm.height), [this, &vm] {
                            Config::setCurrentResolution(
                                window, vm.width, vm.height);
                        });
                    break;

                case 13: // 4:3
                    fourByThree.create<i::Single>(
                        toStr(vm.width) + "x" + toStr(vm.height), [this, &vm] {
                            Config::setCurrentResolution(
                                window, vm.width, vm.height);
                        });
                    break;

                default: // 16:10 and uncommon
                    sixByTen.create<i::Single>(
                        toStr(vm.width) + "x" + toStr(vm.height), [this, &vm] {
                            Config::setCurrentResolution(
                                window, vm.width, vm.height);
                        });
                    break;
            }
        }
    }

    sixByNine.create<i::GoBack>("back");
    fourByThree.create<i::GoBack>("back");
    sixByTen.create<i::GoBack>("back");

    resolution.create<i::Single>(
        "go windowed", [this] { Config::setFullscreen(window, false); });
    resolution.create<i::Single>(
        "go fullscreen", [this] { Config::setFullscreen(window, true); });
    resolution.create<i::GoBack>("back");

    //--------------------------------
    // Graphics

    auto& visfx(optionsMenu.createCategory("visual fxs"));
    gfx.create<i::Goto>("visual fxs", visfx);
    visfx.create<i::Toggle>("3D effects", &Config::get3D, &Config::set3D);
    visfx.create<i::Toggle>(
        "no rotation", &Config::getNoRotation, &Config::setNoRotation) |
        whenNotOfficial;
    visfx.create<i::Toggle>(
        "no background", &Config::getNoBackground, &Config::setNoBackground) |
        whenNotOfficial;
    visfx.create<i::Toggle>(
        "b&w colors", &Config::getBlackAndWhite, &Config::setBlackAndWhite) |
        whenNotOfficial;
    visfx.create<i::Toggle>("pulse", &Config::getPulse, &Config::setPulse) |
        whenNotOfficial;
    visfx.create<i::Toggle>("flash", &Config::getFlash, &Config::setFlash);
    visfx.create<i::GoBack>("back");

    auto& fps(optionsMenu.createCategory("fps settings"));
    gfx.create<i::Goto>("fps settings", fps);
    fps.create<i::Toggle>("vsync", &Config::getVsync,
        [this](bool mValue) { Config::setVsync(window, mValue); });
    fps.create<i::Single>("use static fps", [this] {
        Config::setTimerStatic(window, true);
    }) | whenTimerIsDynamic;
    fps.create<i::Toggle>("limit fps", &Config::getLimitFPS,
        [this](bool mValue) { Config::setLimitFPS(window, mValue); }) |
        whenTimerIsStatic;
    fps.create<i::Slider>(
        "max fps", &Config::getMaxFPS,
        [this](unsigned int mValue) { Config::setMaxFPS(window, mValue); }, 30u,
        200u, 5u) |
        whenTimerIsStatic;
    fps.create<i::Single>("use dynamic fps", [this] {
        Config::setTimerStatic(window, false);
    }) | whenTimerIsStatic;
    fps.create<i::Toggle>("show fps", &Config::getShowFPS, &Config::setShowFPS);
    fps.create<i::GoBack>("back");

    gfx.create<i::Toggle>("text outlines", &Config::getDrawTextOutlines,
        &Config::setDrawTextOutlines);
    gfx.create<i::Slider>(
        "text padding", &Config::getTextPadding,
        [](float mValue) { Config::setTextPadding(mValue); }, 0.f, 64.f, 1.f);
    gfx.create<i::Slider>(
        "text scaling", &Config::getTextScaling,
        [](float mValue) { Config::setTextScaling(mValue); }, 0.1f, 4.f, 0.05f);

    gfx.create<i::Slider>(
        "antialiasing", &Config::getAntialiasingLevel,
        [this](unsigned int mValue) {
            Config::setAntialiasingLevel(window, mValue);
        },
        0u, 3u, 1u);
    gfx.create<i::Toggle>("darken background chunk",
        &Config::getDarkenUnevenBackgroundChunk,
        &Config::setDarkenUnevenBackgroundChunk);
    gfx.create<i::Toggle>(
        "show key icons", &Config::getShowKeyIcons, &Config::setShowKeyIcons);
    gfx.create<i::Slider>(
        "key icons scaling", &Config::getKeyIconsScale,
        [](float mValue) { Config::setKeyIconsScale(mValue); }, 0.1f, 4.f,
        0.05f);

    gfx.create<i::GoBack>("back");

    //--------------------------------
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

    //--------------------------------
    // FRIENDS
    //--------------------------------

    friends.create<i::Single>("add friend", [this] {
        enteredStr = "";
        changeStateTo(States::ETFriend);
    });
    friends.create<i::Single>(
        "clear friends", [this] { assets.pClearTrackedNames(); });
    friends.create<i::GoBack>("back");

    //--------------------------------
    // MAIN MENU
    //--------------------------------

    auto& main{mainMenu.createCategory("main")};
    auto& localProfiles{mainMenu.createCategory("local profiles")};
    main.create<i::Single>("LEVEL SELECT", [this] {
        changeStateTo(States::LevelSelection);
        if(firstLevelSelection)
        {
            lvlDrawer->packIdx = diffMultIdx = 0;
            lvlDrawer->levelDataIds =
                assets.getLevelIdsByPack(assets.getPackInfos().at(0).id);
            setIndex(0);
            firstLevelSelection = false;
        }
        assets.playSound("select.ogg");
    });
    main.create<i::Goto>("LOCAL PROFILES", localProfiles) | whenLocal;
    main.create<i::Single>("OPTIONS", [this] { changeStateTo(States::MOpts); });
    main.create<i::Single>("EXIT", [this] { window.stop(); });

    //--------------------------------
    // PROFILES MENU
    //--------------------------------

    localProfiles.create<i::Single>(
        "CHOOSE PROFILE", [this] { changeStateTo(States::SLPSelect); });
    localProfiles.create<i::Single>("NEW PROFILE", [this] {
        changeStateTo(States::ETLPNew);
        enteredStr = "";
        assets.playSound("select.ogg");
    });
    localProfiles.create<i::GoBack>("BACK");

    //--------------------------------
    // Profiles selection

    auto& profileSelection{
        profileSelectionMenu.createCategory("profile selection")};
    std::string profileName;
    for(auto& p : assets.getLocalProfileNames())
    {
        profileName = p;
        profileSelection.create<i::Single>(profileName,
            [this, profileName] { assets.pSetCurrent(profileName); });
    }
    profileSelection.sortByName();
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
            << "' command line parameter, aborting boot level load\n";

        return false;
    }

    // Iterate through packInfos to find the menu pack index and the index
    // of the level.
    const std::string levelID{packID + "_" + level};
    const auto& p{assets.getPackInfos()};
    const auto& levelsList{assets.getLevelIdsByPack(packID)};

    for(int i{0}; i < static_cast<int>(p.size()); ++i)
    {
        // once you find the pack index search if it contains the level
        if(packID != p.at(i).id)
        {
            continue;
        }

        auto it{std::find(levelsList.begin(), levelsList.end(), levelID)};
        if(it == levelsList.end())
        {

            ssvu::lo("hg::Menugame::MenuGame()")
                << "Invalid level name '" << level
                << "' command line parameter, aborting boot level load\n";

            return false;
        }

        // Level found, initialize parameters
        lvlDrawer->packIdx = i;
        lvlDrawer->levelDataIds = levelsList;
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
    enteredStr = assets.getLocalProfileNames()[0];
    assets.pSetCurrent(enteredStr);
    changeStateTo(States::SMain);

    // Go to level selection
    resetNamesScrolls();
    firstLevelSelection = false;
    changeStateTo(States::LevelSelection);

    // Start game
    window.setGameState(hexagonGame.getGame());
    hexagonGame.newGame(packID,
        lvlDrawer->levelDataIds.at(lvlDrawer->currentIndex), true,
        ssvu::getByModIdx(diffMults, diffMultIdx), false);

    return true;
}

void MenuGame::playLocally()
{
    assets.pSaveCurrent();
    assets.pSetPlayingLocally(true);
    enteredStr = "";
    state = assets.getLocalProfilesSize() == 0 ? States::ETLPNewBoot
                                               : States::SLPSelectBoot;
}

std::pair<const unsigned int, const unsigned int>
MenuGame::pickRandomMainMenuBackgroundStyle()
{
    // If there is no `menubackgrounds.json` abort
    if(!ssvufs::Path{"Assets/menubackgrounds.json"}
            .exists<ssvufs::Type::File>())
    {
        ssvu::lo("::pickRandomMainMenuBackgroundStyle")
            << "File Assets/menubackgrounds.json does not exist" << std::endl;
        return {0, 0};
    }

    std::vector<std::string> levelIDs;
    ssvuj::Obj object = getFromFile("Assets/menubackgrounds.json");
    for(const auto& f : getExtr<vector<string>>(object, "ids"))
    {
        levelIDs.emplace_back(f);
    }

    // pick one of those at random
    const std::string pickedLevel{levelIDs[ssvu::getRndI(0, levelIDs.size())]};

    // retrieve the level index location
    const auto& p(assets.getPackInfos());
    const std::vector<std::string>* levelsIDs;

    // store info main menu requires to set the color theme
    for(int i{0}; i < static_cast<int>(p.size()); ++i)
    {
        levelsIDs = &assets.getLevelIdsByPack(p.at(i).id);
        auto it = find(levelsIDs->begin(), levelsIDs->end(), pickedLevel);
        if(it != levelsIDs->end())
        {
            return {i, it - levelsIDs->begin()};
        }
    }

    return {0, 0};
}

void MenuGame::returnToLevelSelection()
{
    adjustLevelsOffset();
    lvlDrawer->XOffset = 0.f;
    setIgnoreAllInputs(1); // otherwise you go back to the main menu
}

//*****************************************************
//
// NAVIGATION
//
//*****************************************************

void MenuGame::leftAction()
{
    if(state == States::LevelSelection)
    {
        --diffMultIdx;
        difficultyBumpEffect = difficultyBumpEffectMax;
        assets.playSound("difficultyMultDown.ogg");
        touchDelay = 50.f;
        return;
    }

    if(!isInMenu())
    {
        return;
    }

    if(!getCurrentMenu()->getItem().canIncrease())
    {
        return;
    }

    getCurrentMenu()->decrease();
    assets.playSound("beep.ogg");
    touchDelay = 50.f;
}

void MenuGame::rightAction()
{
    if(state == States::LevelSelection)
    {
        ++diffMultIdx;
        difficultyBumpEffect = difficultyBumpEffectMax;
        assets.playSound("difficultyMultUp.ogg");
        touchDelay = 50.f;
        return;
    }

    if(!isInMenu())
    {
        return;
    }

    if(!getCurrentMenu()->getItem().canIncrease())
    {
        return;
    }

    getCurrentMenu()->increase();
    assets.playSound("beep.ogg");
    touchDelay = 50.f;
}

inline constexpr int maxProfilesOnScreen = 6;

void MenuGame::upAction()
{
    if(state == States::LevelSelection)
    {
        if(packChangeState != PackChange::Rest)
        {
            return;
        }

        if(focusHeld)
        {
            changePackQuick(-1);
            return;
        }

        if(getPackInfosSize() == 1)
        {
            setIndex(ssvu::getMod(lvlDrawer->currentIndex - 1, 0,
                lvlDrawer->levelDataIds.size()));
            calcLevelChangeScroll(-2);
        }
        else if(lvlDrawer->currentIndex - 1 < 0)
        {
            // -2 means "go to previous pack and
            // skip to the last level of the list"
            changePackAction(-2);
        }
        else
        {
            setIndex(lvlDrawer->currentIndex - 1);
            calcLevelChangeScroll(-2);
        }

        resetLevelNamesScrolls();
        assets.playSound("beep.ogg");
        touchDelay = 50.f;
        return;
    }

    if(state == States::LoadingScreen)
    {
        if(scrollbarOffset != 0)
        {
            --scrollbarOffset;
            assets.playSound("beep.ogg");
            touchDelay = 50.f;
        }
        return;
    }

    if(!isInMenu())
    {
        return;
    }

    // Scroll the profiles drawn on screen
    if((state == States::SLPSelect || state == States::SLPSelectBoot) &&
        getCurrentMenu()->getIdx() - 1 < scrollbarOffset)
    {
        const int index = ssvu::getMod(getCurrentMenu()->getIdx() - 1, 0,
            static_cast<int>(getCurrentMenu()->getItems().size()));
        scrollbarOffset = std::max(index - (maxProfilesOnScreen - 1), 0);
    }

    do
    {
        getCurrentMenu()->previous();
    }
    while(!getCurrentMenu()->getItem().isEnabled());

    assets.playSound("beep.ogg");
    touchDelay = 50.f;
}

inline constexpr int maxErrorsOnScreen = 7;

void MenuGame::downAction()
{
    if(state == States::LevelSelection)
    {
        if(packChangeState != PackChange::Rest)
        {
            return;
        }

        if(focusHeld)
        {
            changePackQuick(1);
            return;
        }

        if(getPackInfosSize() == 1)
        {
            setIndex(ssvu::getMod(lvlDrawer->currentIndex + 1, 0,
                lvlDrawer->levelDataIds.size()));
            calcLevelChangeScroll(2);
        }
        else if(lvlDrawer->currentIndex + 1 >
            ssvu::toInt(lvlDrawer->levelDataIds.size() - 1))
        {
            changePackAction(1);
        }
        else
        {
            setIndex(lvlDrawer->currentIndex + 1);
            calcLevelChangeScroll(2);
        }

        resetLevelNamesScrolls();
        assets.playSound("beep.ogg");
        touchDelay = 50.f;
        return;
    }

    if(state == States::LoadingScreen)
    {
        if(scrollbarOffset <
            static_cast<int>(loadInfo.errorMessages.size()) - maxErrorsOnScreen)
        {
            ++scrollbarOffset;
            assets.playSound("beep.ogg");
            touchDelay = 50.f;
        }
        return;
    }

    if(!isInMenu())
    {
        return;
    }

    // Scroll the profiles drawn on screen
    if((state == States::SLPSelect || state == States::SLPSelectBoot) &&
        getCurrentMenu()->getIdx() + 1 >
            maxProfilesOnScreen - 1 + scrollbarOffset)
    {
        const int index = ssvu::getMod(getCurrentMenu()->getIdx() + 1, 0,
            static_cast<int>(getCurrentMenu()->getItems().size()));
        scrollbarOffset = std::max(index - (maxProfilesOnScreen - 1), 0);
    }

    do
    {
        getCurrentMenu()->next();
    }
    while(!getCurrentMenu()->getItem().isEnabled());

    assets.playSound("beep.ogg");
    touchDelay = 50.f;
}

void MenuGame::changePack()
{
    const auto& p{assets.getPackInfos()};
    lvlDrawer->packIdx =
        ssvu::getMod(lvlDrawer->packIdx + (packChangeDirection > 0 ? 1 : -1), 0,
            static_cast<int>(p.size()));
    lvlDrawer->levelDataIds =
        assets.getLevelIdsByPack(p.at(lvlDrawer->packIdx).id);
    setIndex(
        packChangeDirection == -2 ? lvlDrawer->levelDataIds.size() - 1 : 0);
    resetNamesScrolls();
}

void MenuGame::changePackQuick(const int direction)
{
    if(isFavoriteLevels())
    {
        return;
    }

    packChangeDirection = direction;
    assets.playSound("beep.ogg");
    changePack();
    adjustLevelsOffset();

    // YOffset is 0 when the first pack is shown and gets lower
    // the further down we have to scroll.

    // Height of the top of the pack label that is one index before the current one.
    float scroll{packLabelHeight * (lvlDrawer->packIdx - 1) + lvlDrawer->YOffset};

    // If the height is lower than the offset of the level selection we must change to
    // that to show the labels before the current one.
    if(scroll < 0.f)
    {
        lvlDrawer->YOffset = std::min(lvlDrawer->YOffset - scroll, 0.f);
        return;
    }

    // Height of the bottom of the pack label that is one index after the current one.
    scroll = packLabelHeight * (lvlDrawer->packIdx + 2) + levelLabelHeight + 3.f * slctFrameSize + lvlDrawer->YOffset;

    // If the height is outside the boundaries of the screen adjust offset to show it.
    if(scroll > h)
    {
        lvlDrawer->YOffset += h - scroll;
    }
}

void MenuGame::changePackAction(const int direction)
{
    if(state != States::LevelSelection || getPackInfosSize() == 1 ||
        packChangeState != PackChange::Rest)
    {
        return;
    }

    lvlDrawer->YScrollTo = 0.f; // stop scrolling for safety
    packChangeState = PackChange::Folding;
    packChangeDirection = direction;
    calcPackChangeScrollSpeed();

    touchDelay = 50.f;
    assets.playSound("beep.ogg");
}

void MenuGame::okAction()
{
    touchDelay = 50.f;

    switch(state)
    {
        case States::ETLPNewBoot:
        case States::ETLPNew:
            if(!enteredStr.empty())
            {
                Category& profiles(profileSelectionMenu.getCategoryByName(
                    "profile selection"));

                // Abort if user is trying to create a profile
                // with a name already in use
                for(auto& i : profiles.getItems())
                {
                    if(enteredStr == i->getName())
                    {
                        assets.playSound("error.ogg");
                        dialogBox.create(
                            "A PROFILE WITH THE SAME NAME ALREADY EXISTS\n"
                            "PLEASE ENTER ANOTHER NAME\n\n"
                            "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n",
                            26, 10.f, DBoxDraw::center);
                        setIgnoreAllInputs(2);
                        return;
                    }
                }

                // All good
                assets.pCreate(enteredStr);
                assets.pSetCurrent(enteredStr);

                // Create new menu item
                std::string name{enteredStr};
                profiles.create<ssvms::Items::Single>(
                    name, [this, name] { assets.pSetCurrent(name); });
                profiles.sortByName();

                enteredStr = "";
                if(state == States::ETLPNewBoot)
                {
                    assets.playSound("openHexagon.ogg");
                    changeStateTo(States::SMain);
                    return;
                }
                changeStateTo(States::SMain);
            }
            break;

        case States::SLPSelectBoot:
            assets.playSound("openHexagon.ogg");
            getCurrentMenu()->exec();
            changeStateTo(States::SMain);
            return;

        case States::SMain:
        {
            const std::string& category{
                getCurrentMenu()->getCategory().getName()};
            getCurrentMenu()->exec();

            if(state == States::LevelSelection)
            {
                adjustLevelsOffset();
                return;
            }

            if(getCurrentMenu() == nullptr)
            {
                return;
            }

            // Scroll to a menu item that is enabled
            getCurrentMenu()->update();
            while(!getCurrentMenu()->getItem().isEnabled())
            {
                getCurrentMenu()->next();
            }

            // Adjust the indents if we moved to a new submenu
            if(getCurrentMenu()->getCategory().getName() != category)
            {
                adjustMenuOffset(true);
            }
        }
        break;

        case States::MOpts:
        {
            getCurrentMenu()->exec();

            // There are two Bind controllers: KeyboardBindControl and
            // JoystickBindControl. So we cast to the common base class to not
            // check for one and the other.
            const auto* const bc{
                dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem())};
            if(bc != nullptr && bc->isWaitingForBind())
            {
                setIgnoreAllInputs(2);
                touchDelay = 10.f;
                assets.playSound("beep.ogg");
                return;
            }

            // Scroll to a menu item that is enabled
            getCurrentMenu()->update();
            while(!getCurrentMenu()->getItem().isEnabled())
            {
                getCurrentMenu()->next();
            }
        }
        break;

        case States::LevelSelection:
            resetNamesScrolls();

            window.setGameState(hexagonGame.getGame());
            hexagonGame.newGame(assets.getPackInfos().at(lvlDrawer->packIdx).id,
                lvlDrawer->levelDataIds.at(lvlDrawer->currentIndex), true,
                ssvu::getByModIdx(diffMults, diffMultIdx),
                false /* executeLastReplay */);
            break;

            /* Currently unused
        case States::ETFriend:
            if(!enteredStr.empty() &&
            !ssvu::contains(assets.pGetTrackedNames(), enteredStr))
            {
                assets.pAddTrackedName(enteredStr);
                changeStateTo(States::SMain);
                enteredStr = "";
            }
            break;

        case States::ETUser:
            if(!enteredStr.empty())
            {
                lrUser = enteredStr;
                changeStateTo(States::ETPass);
                enteredStr = "";
            }
            break;

        case States::ETPass:
            if(!enteredStr.empty())
            {
                lrPass = enteredStr;
                changeStateTo(States::SLogging);
                enteredStr = "";
                //Online::tryLogin(lrUser, lrPass);
            }
            break;

        case States::ETEmail:
            if(!enteredStr.empty() && ssvu::contains(enteredStr, '@'))
            {
                lrEmail = enteredStr;
                enteredStr = "";
                //Online::trySendUserEmail(lrEmail);
            }
            break;
             */

        default:
            if(isInMenu())
            {
                getCurrentMenu()->exec();
            }
            break;
    }

    assets.playSound("select.ogg");
}

void MenuGame::eraseAction()
{
    if(isEnteringText() && !enteredStr.empty())
    {
        enteredStr.erase(enteredStr.end() - 1);
        assets.playSound("beep.ogg");
    }
    else if(state == States::SLPSelect)
    {
        std::string name{
            profileSelectionMenu.getCategory().getItem().getName()};
        // There must be at least one profile, don't erase profile
        // currently in use.
        if(profileSelectionMenu.getCategory().getItems().size() <= 1)
        {
            assets.playSound("error.ogg");
            dialogBox.create(
                "YOU CANNOT ERASE THE ONLY REMAINING PROFILE\n\n"
                "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n",
                26, 10.f, DBoxDraw::center);
            setIgnoreAllInputs(2);
            return;
        }
        if(assets.pGetName() == name)
        {
            assets.playSound("error.ogg");
            dialogBox.create(
                "YOU CANNOT ERASE THE CURRENTLY IN USE PROFILE\n\n"
                "PRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n",
                26, 10.f, DBoxDraw::center);
            setIgnoreAllInputs(2);
            return;
        }

        // Remove the profile .json
        std::string fileName{"Profiles/" + name + ".json"};
        if(std::remove(fileName.c_str()) != 0)
        {
            lo("eraseAction()")
                << "Error: file " << fileName << " does not exist\n";
            return;
        }

        // Remove the item from the menu
        profileSelectionMenu.getCategory().remove();
        assets.playSound("beep.ogg");
    }
    else if(state == States::MOpts && isInMenu())
    {
        auto* const bc{dynamic_cast<BindControlBase*>(&getCurrentMenu()->getItem())};
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
    if(isInMenu() && getCurrentMenu()->getCategory().getName() == "main")
    {
        return;
    }

    assets.playSound("beep.ogg");

    if(state == States::SLPSelectBoot)
    {
        changeStateTo(States::ETLPNewBoot);
        return;
    }

    if(state == States::LevelSelection)
    {
        changeStateTo(States::SMain);
        resetNamesScrolls();
        adjustMenuOffset(false);
        return;
    }

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
                changeStateTo(States::SMain);
            }
            adjustMenuOffset(false);
        }
        else if(isEnteringText())
        {
            changeStateTo(States::SMain);
        }
    }
}

//*****************************************************
//
// UPDATE
//
//*****************************************************

void MenuGame::update(ssvu::FT mFT)
{
    hexagonGame.updateRichPresenceCallbacks();

    hg::Joystick::update();

    if(hg::Joystick::nextPackRisingEdge())
    {
        changePackAction(1);
    }
    else if(hg::Joystick::previousPackRisingEdge())
    {
        changePackAction(-1);
    }

    if(isFavoriteLevels())
    {
        focusHeld = false;
    }
    else if(!focusHeld)
    {
        focusHeld = hg::Joystick::focusPressed();
    }

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

    if(hg::Joystick::screenshotRisingEdge())
    {
        mustTakeScreenshot = true;
    }

    if(touchDelay > 0.f)
    {
        touchDelay -= mFT;
    }

    if(dialogBoxDelay > 0.f)
    {
        dialogBoxDelay -= mFT;
    }

    if(difficultyBumpEffect > 0.f)
    {
        difficultyBumpEffect -= mFT * 2.f;
    }

    if(window.getFingerDownCount() == 1)
    {
        auto wThird{window.getWidth() / 3.f};
        auto wLT{window.getWidth() - wThird};
        auto hThird{window.getHeight() / 3.f};
        auto hLT{window.getHeight() - hThird};

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
    creditsBar2.setTexture(assets.get<sf::Texture>(
        ssvu::getByModIdx(creditsIds, ssvu::toInt(currentCreditsId / 100))));

    /*
    // If connection is lost, kick the player back into welcome screen
    if(!assets.pIsLocal() && Online::getConnectionStatus() != ocs::Connected)
    {
        changeStateTo(States::MWlcm);
    }
     */

    updateLeaderboard();
    updateFriends();

    if(exitTimer > 20)
    {
        window.stop();
    }

    styleData.update(mFT);
    backgroundCamera.turn(levelStatus.rotationSpeed * 10.f);

    if(isEnteringText())
    {
        unsigned int limit{18u};
        for(auto& c : enteredChars)
        {
            if(enteredStr.size() < limit &&
                (ssvu::isAlphanumeric(c) || ssvu::isPunctuation(c)))
            {
                assets.playSound("beep.ogg");
                enteredStr.append(toStr(c));
            }
        }
    }
    enteredChars.clear();

    switch(state)
    {
        case States::LoadingScreen: hexagonRotation += mFT / 100.f; return;

        case States::LevelSelection:
        {
            // Folding animation of the level list when we change pack.
            switch(packChangeState)
            {
                case PackChange::Rest: break;

                case PackChange::Folding:
                    packChangeOffset += mFT * scrollSpeed;
                    if(packChangeOffset < getLevelListHeight())
                    {
                        break;
                    }

                    // Change the pack
                    changePack();
                    // Set the stretch info
                    packChangeOffset = getLevelListHeight();
                    calcPackChangeScrollSpeed();
                    packChangeState = PackChange::Stretching;
                    break;

                case PackChange::Stretching:
                    packChangeOffset -= mFT * scrollSpeed;
                    calcPackChangeScroll();
                    if(packChangeOffset > 0.f)
                    {
                        break;
                    }

                    packChangeOffset = 0.f;
                    adjustLevelsOffset();
                    packChangeState = PackChange::Rest;
                    break;
            }

            const float levelSelectionTotalHeight{getLevelSelectionHeight()};

            // If the height of the list is smaller than the window
            // height the offset of the list is always 0.
            if(levelSelectionTotalHeight < h)
            {
                lvlDrawer->YOffset = lvlDrawer->YScrollTo = 0.f;
                return;
            }

            // This handles the smooth scrolling of the level list
            // when we change level.
            if(lvlDrawer->YScrollTo != 0.f)
            {
                if(lvlDrawer->YOffset < lvlDrawer->YScrollTo)
                {
                    lvlDrawer->YOffset += mFT * scrollSpeed;
                    if(lvlDrawer->YOffset >= lvlDrawer->YScrollTo)
                    {
                        lvlDrawer->YOffset = lvlDrawer->YScrollTo;
                        lvlDrawer->YScrollTo = 0.f;
                    }
                }
                else
                {
                    lvlDrawer->YOffset -= mFT * scrollSpeed;
                    if(lvlDrawer->YOffset <= lvlDrawer->YScrollTo)
                    {
                        lvlDrawer->YOffset = lvlDrawer->YScrollTo;
                        lvlDrawer->YScrollTo = 0.f;
                    }
                }
            }

            // If the list is higher than the screen make sure
            // there is no empty space between the bottom of
            // the list and the bottom of the window.
            const float temp{h - levelSelectionTotalHeight};
            if(lvlDrawer->YOffset <= temp)
            {
                lvlDrawer->YOffset = temp;
            }
        }
            return;

        default: return;
    }
}

void MenuGame::setIndex(const int mIdx)
{
    lvlDrawer->currentIndex = mIdx;

    levelData = &assets.getLevelData(
        lvlDrawer->levelDataIds.at(lvlDrawer->currentIndex));
    formatLevelDescription();

    styleData = assets.getStyleData(levelData->packId, levelData->styleId);
    styleData.computeColors(levelStatus);

    if(isFavoriteLevels())
    {
        const auto& p{assets.getPackInfos()};

        for(int i{0}; i < static_cast<int>(p.size()); ++i)
        {
            if(levelData->packId == p.at(i).id)
            {
                lvlDrawer->packIdx = i;
                break;
            }
        }
    }

    // Set the colors of the menus
    auto& colors{styleData.getColors()};
    menuQuadColor = styleData.getTextColor();
    if(toInt(menuQuadColor.a) == 0)
    {
        for(auto& c : colors)
        {
            if(toInt(c.a) != 0)
            {
                menuQuadColor = c;
                break;
            }
        }
    }
    menuTextColor = colors[0];

    dialogBoxTextColor = menuQuadColor;
    dialogBoxTextColor.a = 255;

    if(colors.size() == 1)
    {
        menuTextColor.a = 255;
        menuSelectionColor = menuQuadColor;
        menuSelectionColor.a = 75;
    }
    else
    {
        if(toInt(menuTextColor.a) == 0 || menuTextColor == menuQuadColor)
        {
            for(auto& c : colors)
            {
                if(toInt(c.a) != 0 && c != menuQuadColor)
                {
                    menuTextColor = c;
                    break;
                }
            }
        }

        menuSelectionColor = colors[1];
        if(toInt(menuSelectionColor.a) == 0 ||
            menuSelectionColor == menuQuadColor ||
            menuSelectionColor == menuTextColor)
        {
            for(auto& c : colors)
            {
                if(toInt(c.a) != 0 && c != menuQuadColor && c != menuTextColor)
                {
                    menuSelectionColor = c;
                    break;
                }
            }
        }
        menuSelectionColor.a = 175;
    }

    txtSelectionBig.font.setFillColor(menuQuadColor);
    txtSelectionSmall.font.setFillColor(menuQuadColor);
    txtSelectionLSmall.font.setFillColor(menuQuadColor);
    txtInstructionsSmall.font.setFillColor(menuQuadColor);
    txtSelectionScore.font.setFillColor(menuQuadColor);

    // Set gameplay values
    diffMults = levelData->difficultyMults;
    diffMultIdx = idxOf(diffMults, 1);

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

// TODO: remove
#if 0
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
#endif
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

    // TODO: remove
#if 0
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
#endif
}

void MenuGame::reloadAssets(const bool reloadEntirePack)
{
    if(state != States::LevelSelection || !dialogBox.empty() ||
        !Config::getDebug())
    {
        return;
    }

    std::string reloadOutput;
    if(reloadEntirePack)
    {
        reloadOutput =
            assets.reloadPack(levelData->packId, levelData->packPath);
    }
    else
    {
        reloadOutput = assets.reloadLevel(
            levelData->packId, levelData->packPath, levelData->id);
    }

    setIndex(lvlDrawer->currentIndex); // loads the new levelData

    reloadOutput += "\nPRESS ANY KEY OR BUTTON TO CLOSE THIS MESSAGE\n";
    uppercasify(reloadOutput);

    // Needs to be two because the dialog box reacts to key releases.
    // First key release is the one of the key press that made the dialog
    // box pop up, the second one belongs to the key press that closes it
    setIgnoreAllInputs(2);
    assets.playSound("select.ogg");
    dialogBox.create(reloadOutput, 26, 10.f, DBoxDraw::center);
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
    txtVersion.font.setString(Config::getVersionString());
    txtVersion.font.setOrigin({getLocalRight(txtVersion.font), 0.f});
    txtVersion.font.setPosition(
        {getGlobalRight(titleBar) - 15.f, getGlobalTop(titleBar) + 15.f});

    creditsBar1.setOrigin({getLocalWidth(creditsBar1), 0.f});
    creditsBar1.setScale({0.373f, 0.373f});
    creditsBar1.setPosition({w - 20.f, 20.f});

    creditsBar2.setOrigin({getLocalWidth(creditsBar2), 0});
    creditsBar2.setScale({0.373f, 0.373f});
    creditsBar2.setPosition({w - 20.f, 17.f + getGlobalBottom(creditsBar1)});

    float scaleFactor{w / 1024.f};
    epilepsyWarning.setOrigin(getLocalCenter(epilepsyWarning));
    epilepsyWarning.setPosition({1024 / (2.f / scaleFactor), 768 / 2.f - 50});
    epilepsyWarning.setScale({0.36f, 0.36f});

    // Readjust the menu background skew and the indents
    fourByThree = 10.f * Config::getWidth() / Config::getHeight() < 16;
    if(fourByThree)
    {
        backgroundCamera.setSkew({1.f, 0.8f});
    }
    else
    {
        backgroundCamera.setSkew({1.f, 0.6f});
    }
    for(auto& c : mainMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }
    for(auto& c : welcomeMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }
    for(auto& c : optionsMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }
    for(auto& c : profileSelectionMenu.getCategories())
    {
        c->getOffset() = 0.f;
    }

    // Update the height infos of the fonts.
    if(fourByThree)
    {
        txtMenuBig.font.setCharacterSize(33);
        txtMenuSmall.font.setCharacterSize(20);
        txtSelectionLSmall.font.setCharacterSize(16);
    }
    else
    {
        txtMenuBig.font.setCharacterSize(45);
        txtMenuSmall.font.setCharacterSize(30);
        txtSelectionLSmall.font.setCharacterSize(24);
    }

    // txtVersion and txtProfile are not in here cause they do not need it.
    for(auto f : {&txtProf, &txtLoadBig, &txtLoadSmall, &txtMenuBig,
            &txtMenuSmall, &txtInstructionsBig, &txtRandomTip,
            &txtInstructionsMedium, &txtInstructionsSmall, &txtEnteringText,
            &txtSelectionBig, &txtSelectionMedium, &txtSelectionLSmall,
            &txtSelectionSmall, &txtSelectionScore})
    {
        f->updateHeight();
    }

    // Readjust the level selection drawing parameters
    updateLevelSelectionDrawingParameters();

    // Reformat the level description, but not on boot.
    // Otherwise the game crashes.
    if(!firstLevelSelection)
    {
        setIndex(lvlDrawer->currentIndex);
    }
}

void MenuGame::refreshBinds()
{
    // Keyboard-mouse
    Trigger triggers[] = {Config::getTriggerRotateCCW(),
        Config::getTriggerRotateCW(), Config::getTriggerFocus(),
        Config::getTriggerSelect(), Config::getTriggerExit(),
        Config::getTriggerForceRestart(), Config::getTriggerRestart(),
        Config::getTriggerReplay(), Config::getTriggerScreenshot(),
        Config::getTriggerSwap(), Config::getTriggerUp(),
        Config::getTriggerDown(), Config::getTriggerNextPack(),
        Config::getTriggerPreviousPack()};

    std::size_t i;
    for(i = 0; i < sizeof(triggers) / sizeof(triggers[0]); ++i)
    {
        game.refreshTrigger(triggers[i], i);
        hexagonGame.refreshTrigger(triggers[i], i);
    }

    // Joystick
    unsigned int buttons[] = {Config::getJoystickSelect(),
        Config::getJoystickExit(), Config::getJoystickFocus(),
        Config::getJoystickSwap(), Config::getJoystickForceRestart(),
        Config::getJoystickRestart(), Config::getJoystickReplay(),
        Config::getJoystickScreenshot(), Config::getJoystickNextPack(),
        Config::getJoystickPreviousPack()};

    for(i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i)
    {
        hg::Joystick::setJoystickBind(buttons[i], i);
    }
}

void MenuGame::setIgnoreAllInputs(const unsigned int presses)
{
    ignoreInputs = presses;

    if(!ignoreInputs)
    {
        game.ignoreAllInputs(false);
        hg::Joystick::ignoreAllPresses(false);
        return;
    }

    game.ignoreAllInputs(true);
    hg::Joystick::ignoreAllPresses(true);
}

//*****************************************************
//
// DRAWING
//
//*****************************************************

void MenuGame::adjustMenuOffset(const bool resetMenuOffset)
{
    if(resetMenuOffset)
    {
        getCurrentMenu()->getCategory().getOffset() = 0.f;
    }

    const auto& items{getCurrentMenu()->getItems()};
    for(auto& i : items)
    {
        i->getOffset() = 0.f;
    }
    items[getCurrentMenu()->getIdx()]->getOffset() = maxOffset;
}

void MenuGame::adjustLevelsOffset()
{
    for(auto& offset : lvlSlct.lvlOffsets)
    {
        offset = 0.f;
    }
    lvlSlct.lvlOffsets[lvlSlct.currentIndex] = maxOffset;

    if(!favSlct.lvlOffsets.size())
    {
        return;
    }
    for(auto& offset : favSlct.lvlOffsets)
    {
        offset = 0.f;
    }
    favSlct.lvlOffsets[favSlct.currentIndex] = maxOffset;
}

inline constexpr float offsetSpeed = 4.f;
inline constexpr float offsetSnap = 0.25f;

float MenuGame::getFPSMult() const
{
    return 200.f / window.getFPS();
}

float MenuGame::calcMenuOffset(float& offset, const float maxOffset,
    const bool revertOffset, const bool speedUp)
{
    float speed;
    if(revertOffset)
    {
        // FPS consistent offset speed
        speed = (speedUp ? 12.f : 8.f) * offsetSpeed * getFPSMult();

        offset = std::max(offset - speed, 0.f);
        if(offset <= offsetSnap)
        {
            offset = 0.f;
        }
    }
    else if(offset < maxOffset)
    {
        speed = (speedUp ? 12.f : 8.f) * offsetSpeed * getFPSMult();
        offset += speed * (1.f - offset / maxOffset);
        if(offset >= maxOffset - offsetSnap)
        {
            offset = maxOffset;
        }
    }
    return maxOffset - offset;
}

void MenuGame::calcMenuItemOffset(float& offset, bool selected)
{
    float speed;
    if(selected)
    {
        if(offset < maxOffset)
        {
            speed = offsetSpeed * getFPSMult();
            offset += speed * (1.f - offset / maxOffset);
            if(offset >= maxOffset - offsetSnap)
            {
                offset = maxOffset;
            }
        }
    }
    else if(offset > 0.f)
    {
        speed = offsetSpeed * getFPSMult();
        offset -= 1.5f * speed * offset / maxOffset;
        if(offset <= offsetSnap)
        {
            offset = 0.f;
        }
    }
}

void MenuGame::createQuad(const sf::Color& color, const float x1,
    const float x2, const float y1, const float y2)
{
    sf::Vector2f topLeft{x1, y1}, topRight{x2, y1}, bottomRight{x2, y2},
        bottomLeft{x1, y2};
    menuQuads.batch_unsafe_emplace_back(
        color, topLeft, topRight, bottomRight, bottomLeft);
}

void MenuGame::createQuadTrapezoid(const sf::Color& color, const float x1,
    const float x2, const float x3, const float y1, const float y2,
    const bool left)
{
    sf::Vector2f topLeft, topRight, bottomRight, bottomLeft;
    if(left)
    {
        topLeft = {x1, y1};
        topRight = {x2, y1};
        bottomRight = {x3, y2};
        bottomLeft = {x1, y2};
    }
    else
    {
        topLeft = {x1, y1};
        topRight = {x2, y1};
        bottomRight = {x2, y2};
        bottomLeft = {x3, y2};
    }
    menuQuads.batch_unsafe_emplace_back(
        color, topLeft, topRight, bottomRight, bottomLeft);
}

std::pair<int, int> MenuGame::getScrollbarNotches(
    const int size, const int maxSize) const
{
    if(size > maxSize)
    {
        return {size - maxSize, maxSize};
    }

    return {0, size};
}
void MenuGame::drawScrollbar(const float totalHeight, const int size,
    const int notches, const float x, const float y, const Color& color)
{
    const float notchHeight{totalHeight / size},
        barHeight{totalHeight - notches * notchHeight},
        startHeight{y + notchHeight * scrollbarOffset};

    menuQuads.clear();
    menuQuads.reserve(4);
    createQuad(
        color, x, x + textToQuadBorder, startHeight, startHeight + barHeight);
    render(menuQuads);
}

void MenuGame::drawMainSubmenus(
    const vector<unique_ptr<Category>>& subMenus, const float indent)
{
    bool currentlySelected, hasOffset;
    for(auto& c : subMenus)
    {
        currentlySelected = mainMenu.getCategory().getName() == c->getName();
        hasOffset = c->getOffset() != 0.f;

        // this submenu has been fully folded
        if(!currentlySelected && !hasOffset)
        {
            continue;
        }

        drawMainMenu(*c, w - indent, !currentlySelected && hasOffset);
    }
}

void MenuGame::drawSubmenusSmall(
    const vector<unique_ptr<Category>>& subMenus, const float indent)
{
    bool currentlySelected, hasOffset;
    for(auto& c : subMenus)
    {
        // Already drawn before this loop
        if(c->getName() == "options")
        {
            continue;
        }

        currentlySelected = optionsMenu.getCategory().getName() == c->getName();
        hasOffset = c->getOffset() != 0.f;

        // this submenu has been fully folded
        if(!currentlySelected && !hasOffset)
        {
            continue;
        }

        drawOptionsSubmenus(*c, indent, !currentlySelected && hasOffset);
    }
}

inline constexpr float fontTopBorder = 0.9f;
inline constexpr float frameSizeMulti = 0.6f;

void MenuGame::drawMainMenu(
    ssvms::Category& mSubMenu, float baseIndent, const bool revertOffset)
{
    const auto& items(mSubMenu.getItems());
    const int size = items.size();

    // Global menu offset
    const float panelOffset{
        calcMenuOffset(mSubMenu.getOffset(), -(baseIndent - w), revertOffset)};
    baseIndent += panelOffset;

    // Calculate quads coordinates.
    // We use font height as our reference parameter.
    const float interline{3.f * txtMenuBig.height},
        quadBorder{txtMenuBig.height * frameSizeMulti},
        doubleBorder{2.f * quadBorder},
        totalHeight{interline * (size - 1) + doubleBorder + txtMenuBig.height};
    float quadHeight{(h - totalHeight) / 2.f + interline - quadBorder},
        txtHeight{quadHeight - txtMenuBig.height * fontTopBorder + quadBorder},
        indent;

    // Store info needed to draw the submenus
    menuHalfHeight = quadHeight + totalHeight / 2.f;

    // Draw the quads that surround the text
    menuQuads.clear();
    menuQuads.reserve(4 * size);

    int i;
    for(i = 0; i < size; ++i)
    {
        calcMenuItemOffset(items[i]->getOffset(), i == mSubMenu.getIdx());
        indent = baseIndent - items[i]->getOffset();

        createQuadTrapezoid(
            !items[i]->isEnabled() ? Color{110, 110, 110, 255} : menuQuadColor,
            indent - txtMenuBig.height * 2.5f, w,
            indent - txtMenuBig.height / 2.f, quadHeight,
            quadHeight + doubleBorder + txtMenuBig.height, false);

        quadHeight += interline;
    }
    render(menuQuads);

    // Draw the text on top of the quads
    std::string itemName;
    for(i = 0; i < size; ++i)
    {
        indent = baseIndent - items[i]->getOffset();
        renderText(items[i]->getName(), txtMenuBig.font, {indent, txtHeight},
            !items[i]->isEnabled() ? Color{150, 150, 150, 255} : menuTextColor);
        txtHeight += interline;
    }
}

void MenuGame::drawOptionsSubmenus(
    ssvms::Category& mSubMenu, const float baseIndent, const bool revertOffset)
{
    const auto& items{mSubMenu.getItems()};
    const int size(items.size());

    // Calculate quads coordinates
    float quadBorder{txtMenuSmall.height * frameSizeMulti};
    const float doubleBorder{quadBorder * 2.f},
        interline{2.5f * txtMenuSmall.height},
        totalHeight{
            interline * (size - 1) + 2.f * doubleBorder + txtMenuSmall.height},
        quadHeight{std::max(menuHalfHeight - totalHeight / 2.f,
            getGlobalBottom(creditsBar2) + 10.f)};

    // Offset
    const float panelOffset{
        calcMenuOffset(mSubMenu.getOffset(), baseIndent, revertOffset, true)};
    const float indent{baseIndent - quadBorder - panelOffset};

    // Draw the quads that surround the text
    menuQuads.clear();
    menuQuads.reserve(8);

    createQuad(menuTextColor, 0, indent + doubleBorder, quadHeight,
        quadHeight + totalHeight);
    createQuad(menuQuadColor, 0, indent + quadBorder, quadHeight + quadBorder,
        quadHeight + totalHeight - quadBorder);
    render(menuQuads);

    // Draw the text on top of the quads
    quadBorder = quadBorder * 1.5f - panelOffset;
    std::string itemName;
    float txtHeight{
        quadHeight - txtMenuSmall.height * fontTopBorder + doubleBorder};
    for(int i{0}; i < size; ++i)
    {
        itemName = items[i]->getName();
        uppercasify(itemName);
        if(i == mSubMenu.getIdx())
        {
            itemName = "> " + itemName;
        }

        renderText(itemName, txtMenuSmall.font, {quadBorder, txtHeight},
            !items[i]->isEnabled() ? Color{150, 150, 150, 255} : menuTextColor);

        txtHeight += interline;
    }
}

std::string MenuGame::formatSurvivalTime(ProfileData* data)
{
    int time{0};
    for(auto& s : data->getScores())
    {
        time += s.asInt();
    }

    std::stringstream stream;
    stream << "Total survival time ";
    if(time < 60)
    {
        stream << std::setfill('0') << std::setw(2) << time;
    }
    else if(time < 3600)
    {
        stream << std::setfill('0') << std::setw(2) << time / 60 << ":"
               << std::setfill('0') << std::setw(2) << time % 60;
    }
    else
    {
        stream << time / 3600 << ":";
        time %= 3600;
        stream << std::setfill('0') << std::setw(2) << time / 60 << ":"
               << std::setfill('0') << std::setw(2) << time % 60;
    }

    return stream.str();
}

inline constexpr float profFrameSize = 10.f;
inline constexpr unsigned int profCharSize = 35;
inline constexpr unsigned int profSelectedCharSize = 35 + 12;

void MenuGame::drawProfileSelection(
    const float xOffset, const bool revertOffset)
{
    if(!assets.pIsLocal())
    {
        throw;
    }

    Category& mSubmenu{profileSelectionMenu.getCategory()};
    const auto& items{mSubmenu.getItems()};

    const int realSize(items.size());
    auto [scrollbarNotches, drawnSize] =
        getScrollbarNotches(realSize, maxProfilesOnScreen);

    // Calculate the height
    const float fontHeight{getFontHeight(txtProfile.font, profCharSize)},
        selectedFontHeight{
            getFontHeight(txtProfile.font, profSelectedCharSize)};

    // check if the width of the menu should be increased
    constexpr float profMinWidth = 400.f;
    float textWidth{profMinWidth};
    std::string itemName;
    for(auto& p : items)
    {
        itemName = p->getName();
        uppercasify(itemName);
        txtProfile.font.setString(itemName);
        textWidth = std::max(textWidth, getGlobalWidth(txtProfile.font));
    }

    // Calculate horizontal coordinates
    constexpr float profMinHeight = 360.f;
    const float interline{4.f * fontHeight}, doubleBorder{profFrameSize * 2.f},
        totalHeight{std::max(
            interline * (drawnSize - 1) + doubleBorder * 2.f + fontHeight * 3.f,
            profMinHeight)};

    // always account for the scrollbar space
    constexpr float scrollbarInterspace = 3.f;
    textWidth += doubleBorder + scrollbarInterspace * 2.f;

    // Make sure the box does not go out of bounds
    float indent{((w + xOffset) * 0.5f - textWidth) / 2.f + profFrameSize};
    indent = std::max(indent, doubleBorder);

    // Make sure the instructions do not go out of bounds
    txtInstructionsSmall.font.setString(
        "Press backspace to delete the selected profile\n"
        "You cannot delete the profile currently in use");
    const float instructionsWidth{getGlobalWidth(txtInstructionsSmall.font)},
        resultIndent{indent + (textWidth - instructionsWidth) / 2.f};
    if(resultIndent < 0.f)
    {
        indent += -resultIndent + 10.f;
    }

    // Calculate vertical coordinates
    float quadHeight{
        std::max((h - totalHeight) / 2.f, getGlobalBottom(titleBar) + 60.f)},
        txtHeight{quadHeight - fontHeight * fontTopBorder + doubleBorder +
                  profFrameSize * 0.5f};

    // Submenu global offset
    const float panelOffset{
        calcMenuOffset(mSubmenu.getOffset(), h - quadHeight, revertOffset)};
    txtHeight += panelOffset;
    quadHeight += panelOffset;

    // Draw the quads that surround the text and the scroll bar if needed
    menuQuads.clear();
    menuQuads.reserve(8);

    createQuad(menuTextColor, indent - doubleBorder,
        indent + doubleBorder + textWidth, quadHeight,
        quadHeight + totalHeight);

    createQuad(menuQuadColor, indent - profFrameSize,
        indent + profFrameSize + textWidth, quadHeight + profFrameSize,
        quadHeight + totalHeight - profFrameSize);

    render(menuQuads);

    if(scrollbarNotches != 0)
    {
        drawScrollbar(totalHeight - 2.f * (profFrameSize + scrollbarInterspace),
            realSize, scrollbarNotches,
            indent + textWidth - scrollbarInterspace,
            quadHeight + profFrameSize + scrollbarInterspace, menuTextColor);
    }

    // Draw the text on top of the quads
    float yPos;
    bool selected;
    ProfileData* data;
    txtHeight += profFrameSize / 2.f;
    for(int i{scrollbarOffset}; i < drawnSize + scrollbarOffset; ++i)
    {
        selected = i == mSubmenu.getIdx();

        // Draw profile name
        itemName = items[i]->getName();
        uppercasify(itemName);
        yPos = txtHeight - (selected ? fontHeight * 0.75f : 0.f);
        renderTextCentered(itemName, txtProfile.font,
            selected ? profSelectedCharSize : profCharSize,
            {indent + textWidth / 2.f, yPos}, menuTextColor);

        // Add total survival time for extra flavor
        data = assets.getLocalProfileByName(itemName);
        if(data != nullptr)
        {
            yPos += (selected ? selectedFontHeight : fontHeight) * 1.75f;
            txtProfile.font.setCharacterSize(
                txtProfile.font.getCharacterSize() - 15);
            renderTextCentered(formatSurvivalTime(data), txtProfile.font,
                {indent + textWidth / 2.f, yPos});
        }

        txtHeight += interline;
    }

    // Add message about profile deletion
    txtInstructionsSmall.font.setPosition(
        {indent + (textWidth - instructionsWidth) / 2.f,
            quadHeight + totalHeight});
    render(txtInstructionsSmall.font);
}

void MenuGame::drawProfileSelectionBoot()
{
    if(!assets.pIsLocal())
    {
        throw;
    }

    Category& mSubmenu{profileSelectionMenu.getCategory()};
    const auto& items(mSubmenu.getItems());

    const int realSize(items.size());
    auto [scrollbarNotches, drawnSize] =
        getScrollbarNotches(realSize, maxProfilesOnScreen);

    const float fontHeight{getFontHeight(txtProfile.font, profCharSize)},
        selectedFontHeight{
            getFontHeight(txtProfile.font, profSelectedCharSize)};

    // Calculate coordinates
    const float interline{4.f * fontHeight}, totalHeight{interline * drawnSize};
    float height{(h - totalHeight) / 2.f - selectedFontHeight * 1.5f};

    // Draw instructions
    const float instructionsHeight{1.5f * txtInstructionsBig.height};
    // Make sure the instructions do not overlap the title bar or the credits
    height = std::max(
        height - 2.f * instructionsHeight, getGlobalBottom(titleBar) + 40.f);

    const std::string instructions[] = {
        "SELECT LOCAL PROFILE", "PRESS ESC TO CREATE A NEW PROFILE"};
    for(auto& s : instructions)
    {
        renderTextCentered(s, txtInstructionsBig.font, {w / 2.f, height});
        height += instructionsHeight;
    }
    height += selectedFontHeight;

    // Draw scrollbar if needed
    std::string itemName;
    if(scrollbarNotches != 0)
    {
        float width;
        for(auto& p : items)
        {
            itemName = p->getName();
            uppercasify(itemName);
            txtProfile.font.setString(itemName);
            width = std::max(width, getGlobalWidth(txtProfile.font));
        }
        txtProfile.font.setString("Total survival time 0000:00");
        width = std::max(width, getGlobalWidth(txtProfile.font));
        width += 10.f;

        drawScrollbar(totalHeight, realSize, scrollbarNotches,
            (w + width) / 2.f, height, Color::White);
    }

    // Draw profile names and score
    bool selected;
    float yPos;
    ProfileData* data;
    for(int i{scrollbarOffset}; i < drawnSize + scrollbarOffset; ++i)
    {
        selected = i == mSubmenu.getIdx();

        // Draw profile name
        itemName = items[i]->getName();
        uppercasify(itemName);
        yPos = height - (selected ? fontHeight * 0.75f : 0.f);
        renderTextCentered(itemName, txtProfile.font,
            selected ? profSelectedCharSize : profCharSize, {w / 2.f, yPos});

        // Add total survival time for extra flavor
        data = assets.getLocalProfileByName(itemName);
        if(data != nullptr)
        {
            yPos += (selected ? selectedFontHeight : fontHeight) * 1.75f;
            txtProfile.font.setCharacterSize(
                txtProfile.font.getCharacterSize() - 15);
            renderTextCentered(
                formatSurvivalTime(data), txtProfile.font, {w / 2.f, yPos});
        }

        height += interline;
    }
}

void MenuGame::drawEnteringText(const float xOffset, const bool revertOffset)
{
    // Set text parameters
    uppercasify(enteredStr);
    txtEnteringText.font.setString(enteredStr);
    constexpr float enteringTextMinWidth = 200.f;
    const float textWidth{
        std::max(enteringTextMinWidth, getGlobalWidth(txtEnteringText.font))};

    // Calculate coordinates
    const float doubleFrame{profFrameSize * 2.f},
        indent{((w + xOffset) * 0.5f - textWidth) / 2.f + profFrameSize},
        txtBottom{txtEnteringText.height * 0.45f},
        totalHeight{txtEnteringText.height + txtBottom + doubleFrame * 2.f};
    float quadHeight{menuHalfHeight - totalHeight / 2.f},
        txtHeight{
            quadHeight - txtEnteringText.height * fontTopBorder + doubleFrame};

    // Offset
    const float panelOffset{
        calcMenuOffset(enteringTextOffset, h - quadHeight, revertOffset)};
    txtHeight += panelOffset;
    quadHeight += panelOffset;

    // Draw the quads that surround the text
    menuQuads.clear();
    menuQuads.reserve(8);

    createQuad(menuTextColor, indent - doubleFrame,
        indent + doubleFrame + textWidth, quadHeight, quadHeight + totalHeight);

    createQuad(menuQuadColor, indent - profFrameSize,
        indent + profFrameSize + textWidth, quadHeight + profFrameSize,
        quadHeight + totalHeight - profFrameSize);

    render(menuQuads);

    // Draw the text on top of the quads
    renderTextCenteredOffset(enteredStr, txtEnteringText.font,
        {textWidth / 2.f, txtHeight + profFrameSize / 2.f}, indent,
        menuTextColor);

    // Draw instructions text above the quads
    const std::string instructions[] = {
        "INSERT TEXT", "PRESS ENTER WHEN DONE", "PRESS ESC TO ABORT"};
    const float instructionsHeight{txtInstructionsMedium.height * 1.5f};
    txtHeight -= profFrameSize + instructionsHeight * 3.f;
    txtInstructionsMedium.font.setFillColor(menuQuadColor);
    for(auto& s : instructions)
    {
        renderTextCenteredOffset(s, txtInstructionsMedium.font,
            {textWidth / 2.f, txtHeight}, indent);
        txtHeight += instructionsHeight;
    }
}

void MenuGame::drawEnteringTextBoot()
{
    // Set parameters
    float height{(h - txtEnteringText.height) / 2};

    // Draw instructions text
    const float instructionsHeight{txtInstructionsBig.height * 1.5f};
    height -= instructionsHeight * 2.f;
    const std::string instructions[] = {
        "PROFILE CREATION", "PLEASE TYPE A NAME AND PRESS ENTER"};
    for(auto& s : instructions)
    {
        renderTextCentered(s, txtInstructionsBig.font, {w / 2.f, height});
        height += instructionsHeight;
    }

    // Draw the entered name text
    height += instructionsHeight / 2.f;
    uppercasify(enteredStr);
    renderTextCentered(
        enteredStr, txtEnteringText.font, {w / 2.f, height}, Color::White);
}

void MenuGame::drawLoadResults()
{
    //--------------------------------------
    // Hexagon
    const float div{ssvu::tau / 6 * 0.5f}, hexagonRadius{100.f};
    const sf::Vector2f centerPos = {w / 2.f, h / 5.f};

    menuQuads.clear();

    int i;
    menuQuads.reserve(4 * 6);
    for(i = 0; i < 6; ++i)
    {
        const float sAngle{div * 2.f * (i + hexagonRotation)};

        const sf::Vector2f topLeft{
            ssvs::getOrbitRad(centerPos, sAngle - div, hexagonRadius)};
        const sf::Vector2f topRight{
            ssvs::getOrbitRad(centerPos, sAngle + div, hexagonRadius)};
        const sf::Vector2f bottomRight{
            ssvs::getOrbitRad(centerPos, sAngle + div, hexagonRadius + 10.f)};
        const sf::Vector2f bottomLeft{
            ssvs::getOrbitRad(centerPos, sAngle - div, hexagonRadius + 10.f)};

        menuQuads.batch_unsafe_emplace_back(
            Color::White, topLeft, topRight, bottomRight, bottomLeft);
    }

    //--------------------------------------
    // Vertical separators

    menuQuads.reserve_more(4 * 3);
    const float xOffset{w / 4.f};
    float topHeight{h / 2.f - h / 15.f}, bottomHeight{h / 2.f + h / 15.f};

    for(i = -1; i < 2; ++i)
    {
        createQuad(Color::White, w / 2.f + i * xOffset - 5.f,
            w / 2.f + i * xOffset + 5.f, topHeight, bottomHeight);
    }

    render(menuQuads);

    //--------------------------------------
    // Counters: text and numbers

    topHeight += 5.f - txtLoadSmall.height;
    const float numbersHeight = bottomHeight - txtLoadBig.height * 2.1f;

    txtLoadSmall.font.setFillColor(Color::White);
    txtLoadBig.font.setFillColor(Color::White);

    // 1
    float textOffset{w - 3.f * xOffset};
    renderTextCentered(
        "PACKS LOADED", txtLoadSmall.font, {textOffset / 2.f, topHeight});
    renderTextCentered(toStr(loadInfo.packs), txtLoadBig.font,
        {textOffset / 2.f, numbersHeight});

    // 2
    textOffset = w - xOffset;
    renderTextCentered(
        "LEVELS LOADED", txtLoadSmall.font, {textOffset / 2.f, topHeight});
    renderTextCentered(toStr(loadInfo.levels), txtLoadBig.font,
        {textOffset / 2.f, numbersHeight});

    // 3
    textOffset = w + xOffset;
    renderTextCentered(
        "ASSETS LOADED", txtLoadSmall.font, {textOffset / 2.f, topHeight});
    renderTextCentered(toStr(loadInfo.assets), txtLoadBig.font,
        {textOffset / 2.f, numbersHeight});

    //--------------------------------------
    // Random tip

    const float tipInterline{txtRandomTip.height * 1.5f};
    float height{h - tipInterline * 2.f};
    for(i = 1; i >= 0; --i) // all tips are on two lines
    {
        renderTextCentered(std::string(randomTip[i]), txtRandomTip.font,
            {w / 2.f, height - tipInterline});
        height -= tipInterline;
    }

    //--------------------------------------
    // Errors (if any)

    int size = loadInfo.errorMessages.size();

    textOffset = w + 3.f * xOffset;
    renderTextCentered(
        "ERRORS", txtLoadSmall.font, {textOffset / 2.f, topHeight}, Color::Red);
    renderTextCentered(toStr(size), txtLoadBig.font,
        {textOffset / 2.f, numbersHeight}, Color::Red);

    // No error messages
    if(!size)
    {
        bottomHeight += txtLoadSmall.height * 1.75f;
        renderTextCentered("NO LOAD ERRORS", txtLoadSmall.font,
            {w / 2.f, bottomHeight}, Color::White);
        return;
    }

    // Print errors from last to first.
    bottomHeight += txtLoadSmall.height / 2.f;
    const float txtSpacing{txtLoadSmall.height * 2.f};

    // But first handle the scrollbar
    auto [scrollbarNotches, drawnSize] =
        getScrollbarNotches(size, maxErrorsOnScreen);
    if(scrollbarNotches != 0)
    {
        drawScrollbar(txtSpacing * drawnSize, size, scrollbarNotches,
            w - 4.f - textToQuadBorder, bottomHeight + txtSpacing / 4.f,
            Color::Red);
    }

    for(i = drawnSize - 1 + scrollbarOffset; i > -1 + scrollbarOffset; --i)
    {
        renderTextCentered(loadInfo.errorMessages[i], txtLoadSmall.font,
            {w / 2.f, bottomHeight});
        bottomHeight += txtSpacing;
    }
}

void MenuGame::updateLevelSelectionDrawingParameters()
{
    textToQuadBorder = txtSelectionMedium.height * frameSizeMulti;
    slctFrameSize = textToQuadBorder * 0.3f;
    packLabelHeight =
        txtSelectionMedium.height + 2.f * textToQuadBorder + slctFrameSize;
    levelLabelHeight = txtSelectionBig.height +           // level name
                       txtSelectionSmall.height * 1.75f + // author + interspace
                       2.f * textToQuadBorder -
                       slctFrameSize; // top and bottom spaces
}
float MenuGame::getLevelListHeight() const
{
    return levelLabelHeight * lvlDrawer->levelDataIds.size() + slctFrameSize;
}
float MenuGame::getLevelSelectionHeight() const
{
    return packLabelHeight * getPackInfosSize() +
           levelLabelHeight * (focusHeld ? 1 : lvlDrawer->levelDataIds.size()) -
           packChangeOffset +
           (lvlDrawer->packIdx != static_cast<int>(getPackInfosSize()) - 1
                   ? 3.f
                   : 2.f) *
               slctFrameSize;
}

void MenuGame::scrollName(std::string& text, float& scroller)
{
    // FPS consistent scrolling
    scroller += getFPSMult();
    text += "  ";

    auto it{std::next(
        text.begin(), ssvu::getMod(toInt(scroller / 100.f), text.length()))};
    std::string charsToMove;
    std::move(text.begin(), it, std::back_inserter(charsToMove));
    text.erase(text.begin(), it);
    text += charsToMove;
}
void MenuGame::scrollNameRightBorder(std::string& text, const std::string key,
    sf::Text& font, float& scroller, float border)
{
    // Store length of the key
    font.setString(key);
    const float keyWidth = getGlobalWidth(font);

    Utils::uppercasify(text);
    font.setString(text);

    // If the text is already within border format and return
    border -= keyWidth;
    if(getGlobalWidth(font) <= border)
    {
        text = key + text;
        return;
    }

    // Scroll the name and shrink it to the required length
    scrollName(text, scroller);
    font.setString(text);
    while(getGlobalWidth(font) > border && text.length() > 1)
    {
        text.pop_back();
        font.setString(text);
    }
    text = key + text;
}
void MenuGame::scrollNameRightBorder(
    std::string& text, sf::Text& font, float& scroller, const float border)
{
    Utils::uppercasify(text);
    font.setString(text);

    if(getGlobalWidth(font) <= border)
    {
        return;
    }

    scrollName(text, scroller);
    font.setString(text);
    while(getGlobalWidth(font) > border && text.length() > 1)
    {
        text.pop_back();
        font.setString(text);
    }
}

void MenuGame::resetNamesScrolls()
{
    for(int i = 0; i < static_cast<int>(Label::ScrollsSize); ++i)
    {
        namesScroll[i] = 0;
    }
}
void MenuGame::resetLevelNamesScrolls()
{
    // Reset all scrolls except the ones relative to the pack.
    namesScroll[static_cast<int>(Label::LevelName)] = 0.f;
    for(int i = static_cast<int>(Label::MusicName);
        i < static_cast<int>(Label::ScrollsSize); ++i)
    {
        namesScroll[i] = 0.f;
    }
}

inline constexpr float baseScrollSpeed = 45.f;

void MenuGame::calcLevelChangeScroll(const int dir)
{
    scrollSpeed = baseScrollSpeed;

    float scroll;
    if(dir < 0)
    {
        // If we are approaching the top of the pack show either the first
        // level label and the next pack label or two previous pack labels.
        if(lvlDrawer->currentIndex < 2)
        {
            scroll = packLabelHeight * (lvlDrawer->packIdx + 1 -
                                           (2 - lvlDrawer->currentIndex)) +
                     lvlDrawer->YOffset;
        }
        else
        {
            //...otherwise just show the two previous level labels.
            scroll = packLabelHeight * (lvlDrawer->packIdx + 1) +
                     levelLabelHeight * (lvlDrawer->currentIndex + dir) +
                     slctFrameSize + lvlDrawer->YOffset;
        }

        if(scroll < 0.f)
        {
            // std::min prevents scrolling above the top of the list if we are
            // in the first pack.
            lvlDrawer->YScrollTo = std::min(lvlDrawer->YOffset - scroll, 0.f);
        }
        return;
    }

    const int size = lvlDrawer->levelDataIds.size();
    // If we are approaching the bottom of the pack show either the
    // last level label and the next pack label or two next pack labels...
    if(lvlDrawer->currentIndex >= size - 2)
    {
        scroll =
            packLabelHeight * (lvlDrawer->packIdx + 1 +
                                  (2 - (size - 1 - lvlDrawer->currentIndex))) +
            levelLabelHeight * size + 3.f * slctFrameSize + lvlDrawer->YOffset;
    }
    else
    {
        //...otherwise just show the two next level labels.
        scroll = packLabelHeight * (lvlDrawer->packIdx + 1) +
                 levelLabelHeight * (lvlDrawer->currentIndex + dir + 1) +
                 2.f * slctFrameSize + lvlDrawer->YOffset;
    }

    if(scroll > h)
    {
        lvlDrawer->YScrollTo = lvlDrawer->YOffset + h - scroll;
    }
}
void MenuGame::calcPackChangeScroll()
{
    float scrollTop, scrollBottom;
    if(packChangeDirection == -2)
    {
        // Handles switching from the first level of a pack to the last
        // level of the previous packs. Show the level + the two next
        // pack labels (if they exist).
        scrollTop = packLabelHeight * (lvlDrawer->packIdx + 1 + 2) +
                    slctFrameSize + levelLabelHeight * lvlDrawer->currentIndex +
                    lvlDrawer->YOffset;
        scrollBottom = scrollTop + levelLabelHeight + slctFrameSize * 2;
    }
    else
    {
        // The list is shifted to try fit all levels in the pack.
        // If that is not possible just include the pack label
        // + whatever amount of levels it's possible to fit on screen.
        const float levelsListHeight{
            std::min(packLabelHeight + 2.f * slctFrameSize +
                         levelLabelHeight * lvlDrawer->levelDataIds.size(),
                h) -
            levelLabelHeight};
        scrollTop = packLabelHeight * lvlDrawer->packIdx + lvlDrawer->YOffset +
                    levelsListHeight;
        scrollBottom = scrollTop + levelLabelHeight;
    }

    if(scrollBottom > h)
    {
        lvlDrawer->YScrollTo = lvlDrawer->YOffset + h - scrollBottom;
    }
    else if(scrollTop < 0.f)
    {
        lvlDrawer->YScrollTo = lvlDrawer->YOffset - scrollTop;
    }
}
void MenuGame::calcPackChangeScrollSpeed()
{
    // Only speed up the animation if there are more than 12 levels.
    scrollSpeed =
        baseScrollSpeed * std::max(lvlDrawer->levelDataIds.size() / 12.f, 1.f);
}

float MenuGame::getMaximumTextWidth() const
{
    return w * 0.33f - 2.f * textToQuadBorder;
}

inline constexpr int descLines = 7;

void MenuGame::formatLevelDescription()
{
    levelDescription.clear();
    std::string desc{
        assets.getLevelData(lvlDrawer->levelDataIds.at(lvlDrawer->currentIndex))
            .description};

    if(desc.empty())
    {
        return;
    }
    uppercasify(desc);

    // Split description into words.
    desc += '\n'; // Add a safety newline
    std::size_t i{0}, j{0};
    std::vector<std::string> words;
    for(; i < desc.size(); ++i)
    {
        if(desc[i] == '\n')
        {
            words.emplace_back(desc.substr(j, i - j + 1)); // include newline.
            j = i + 1;
        }
        else if(desc[i] == ' ')
        {
            words.emplace_back(desc.substr(j, i - j));
            j = i + 1; // skip the space.
        }
    }

    // Group words into lines depending on wherever
    // they fit within the maximum width.
    const float maxWidth{getMaximumTextWidth()};
    std::string candidate, temp;
    for(i = 0; i < words.size() && levelDescription.size() < descLines; ++i)
    {
        if(!candidate.empty())
        {
            temp = " " + words[i];
            txtSelectionSmall.font.setString(candidate + temp);
        }
        else
        {
            temp = words[i];
            txtSelectionSmall.font.setString(temp);
        }

        // If last character is a newline...
        if(temp[temp.size() - 1] == '\n')
        {
            // ...if it all fits add to the vector as a single line...
            if(getGlobalWidth(txtSelectionSmall.font) < maxWidth)
            {
                candidate += temp;
                levelDescription.push_back(candidate);
            }
            else
            {
                // ...otherwise add "candidate" to the vector and add the new
                // word on its own.
                levelDescription.push_back(candidate);
                if(levelDescription.size() < descLines)
                {
                    levelDescription.push_back(words[i]);
                }
            }
            candidate.clear();
            continue;
        }

        // If there is no newline check if the line fits...
        if(getGlobalWidth(txtSelectionSmall.font) < maxWidth)
        {
            candidate += temp;
            continue;
        }

        // ...if it doesn't add to the vector "candidate" and then set it to be
        // just the overflowing word, to be checked upon in the next cycles.
        levelDescription.push_back(candidate);
        candidate = words[i];
    }

    // Add whatever is left if it fits.
    if(levelDescription.size() < descLines)
    {
        levelDescription.push_back(candidate);
    }
}

void MenuGame::changeLevelFavoriteFlag()
{
    const LevelData& data{assets.getLevelData(
        lvlDrawer->levelDataIds.at(lvlDrawer->currentIndex))};
    const std::string& levelID{data.packId + "_" + data.id};

    // Level is a favorite so remove it.
    if(data.favorite)
    {
        favSlct.levelDataIds.erase(std::find(
            favSlct.levelDataIds.begin(), favSlct.levelDataIds.end(), levelID));
        favSlct.lvlOffsets.pop_back();
        assets.setLevelFavoriteFlag(levelID, false);

        // If the favorite levels vector is empty
        // after the removal force exit from the
        // favorites menu.
        if(favSlct.levelDataIds.empty())
        {
            favSlct.currentIndex = 0;
            favSlct.XOffset = 0.f; // this way the menu is not drawn anymore.
            lvlDrawer = &lvlSlct;
            setIndex(lvlSlct.currentIndex);
        }
        else if(isFavoriteLevels())
        {
            // If not empty make sure the selected level
            // is one of the remaining ones.
            lvlDrawer->currentIndex = std::min(lvlDrawer->currentIndex,
                static_cast<int>(favSlct.levelDataIds.size()) - 1);
        }
        adjustLevelsOffset();
    }
    else
    {
        // Add the level to the favorites.
        favSlct.levelDataIds.emplace_back(levelID);
        favSlct.lvlOffsets.emplace_back(0.f);
        assets.setLevelFavoriteFlag(levelID, true);

        // Sort in alphabetical order.
        std::sort(favSlct.levelDataIds.begin(), favSlct.levelDataIds.end(),
            [this](const std::string& a, const std::string& b) -> bool {
                return assets.getLevelData(a).name <
                       assets.getLevelData(b).name;
            });
    }

    assets.playSound("select.ogg");
}

void MenuGame::switchToFromFavoriteLevels()
{
    if(state != States::LevelSelection || favSlct.levelDataIds.empty())
    {
        return;
    }

    // Quickly finish any ongoing pack changes.
    if(packChangeState == PackChange::Folding)
    {
        changePack();
    }
    packChangeState = PackChange::Rest;
    packChangeOffset = 0.f;

    lvlDrawer = isFavoriteLevels() ? &lvlSlct : &favSlct;
    setIndex(lvlDrawer->currentIndex); // update the looks
    adjustLevelsOffset();
    resetNamesScrolls();
    assets.playSound("select.ogg");
}

void MenuGame::saveFavoriteLevels()
{
    ssvuj::Obj root;
    ssvuj::arch(root, "ids", favSlct.levelDataIds);
    ssvuj::writeToFile(root, std::string(favoritePath));
}

void MenuGame::drawLevelSelectionRightSide(
    LevelDrawer& drawer, const bool revertOffset)
{
    const float outerFrame{textToQuadBorder + slctFrameSize},
        sidepanelIndent{w * 0.33f - outerFrame}, quadsIndent{w - sidepanelIndent},
        txtIndent{w - sidepanelIndent / 2.f},
        levelIndent{quadsIndent + outerFrame},
        rightSideOffset{
            calcMenuOffset(drawer.XOffset, w - quadsIndent, revertOffset)};
    const auto& infos{assets.getPackInfos()};
    int packsSize, levelsSize;
    if(drawer.isFavorites)
    {
        levelsSize = drawer.levelDataIds.size();
        packsSize = 1;
    }
    else
    {
        packsSize = infos.size();
        levelsSize = focusHeld ? 1 : drawer.levelDataIds.size();
    }
    const LevelData* levelDataTemp;
    std::string tempString;
    float prevLevelIndent{0.f}, height{0.f}, indent, tempFloat;
    sf::Vector2f topLeft, topRight, bottomRight, bottomLeft;

    // The drawing order is: levels list then pack labels.
    // The reason for it is that when a pack is deselected the
    // level list slides up and it must do so below the previous pack labels.
    // Therefore pack labels must be drawn above everything else (aka must
    // be drawn last).

    renderTextCentered(
        isFavoriteLevels() ? "PRESS F2 TO SHOW ALL LEVELS" :
                             "PRESS F2 TO SHOW FAVORITE LEVELS",
        txtSelectionSmall.font, {w / 2.f, 5.f});

    //----------------------------------------
    // LEVELS LIST

    int i;
    Color alphaTextColor{
        menuTextColor.r, menuTextColor.g, menuTextColor.b, 150};
    txtSelectionMedium.font.setFillColor(menuTextColor);
    height = packLabelHeight * (isFavoriteLevels() ? 1 : drawer.packIdx + 1) + slctFrameSize -
             packChangeOffset + drawer.YOffset;

    for(i = 0; i < levelsSize; ++i)
    {
        //-------------------------------------
        // Quads
        menuQuads.clear();
        menuQuads.reserve(12);

        // If the list is folding give all level labels the same alignment
        if(packChangeState != PackChange::Rest)
        {
            drawer.lvlOffsets[i] = 0.f;
        }
        else
        {
            calcMenuItemOffset(drawer.lvlOffsets[i], i == drawer.currentIndex);
        }
        indent = quadsIndent;
        if(!focusHeld)
        {
            indent -= drawer.lvlOffsets[i];
        }
        tempFloat = indent + rightSideOffset;

        // Top frame
        if(i > 0 && drawer.lvlOffsets[i - 1] > drawer.lvlOffsets[i])
        {
            createQuad(menuQuadColor, prevLevelIndent, w, height,
                height + slctFrameSize);
        }
        else
        {
            createQuad(
                menuQuadColor, tempFloat, w, height, height + slctFrameSize);
        }
        // Side frame
        createQuad(menuQuadColor, tempFloat, tempFloat + slctFrameSize,
            height + slctFrameSize, height + levelLabelHeight);
        // Body
        createQuad(
            i == drawer.currentIndex ? menuSelectionColor : alphaTextColor,
            tempFloat + slctFrameSize, w, height + slctFrameSize,
            height + levelLabelHeight);

        render(menuQuads);
        prevLevelIndent = tempFloat;

        //-------------------------------------
        // Level data
        levelDataTemp = &assets.getLevelData(drawer.levelDataIds.at(i));
        if(levelDataTemp == nullptr)
        {
            continue;
        }

        //-------------------------------------
        // Level name

        indent = levelIndent;
        if(!focusHeld)
        {
            indent -= drawer.lvlOffsets[i];
        }
        tempFloat = indent + rightSideOffset;
        height += textToQuadBorder;

        tempString = focusHeld ? "..." : levelDataTemp->name;
        uppercasify(tempString);
        renderText(tempString, txtSelectionBig.font,
            {tempFloat, height - txtSelectionBig.height * fontTopBorder});

        //-------------------------------------
        // Author
        height += txtSelectionBig.height + txtSelectionSmall.height * 0.75f;

        tempString = focusHeld ? "..." : levelDataTemp->author;
        uppercasify(tempString);
        renderText(tempString, txtSelectionSmall.font,
            {tempFloat, height - txtSelectionSmall.height * 0.7f});

        height += txtSelectionSmall.height + textToQuadBorder - slctFrameSize;
    }

    // Bottom frame for the last element
    menuQuads.clear();
    menuQuads.reserve(4);
    createQuad(
        menuQuadColor, prevLevelIndent, w, height, height + slctFrameSize);
    render(menuQuads);

    height += slctFrameSize;
    i = ssvu::getMod(drawer.packIdx + 1, packsSize);
    if(i == 0)
    {
        height = drawer.YOffset;
    }

    //----------------------------------------
    // PACKS LABELS

    const float arrowWidth{packLabelHeight / 2.f - textToQuadBorder};

    do
    {
        // Quads
        menuQuads.clear();
        menuQuads.reserve(8);

        tempFloat = quadsIndent - outerFrame + rightSideOffset;
        createQuad(menuTextColor, tempFloat - slctFrameSize, w, height,
            height + txtSelectionMedium.height + 2.f * outerFrame);
        createQuad(menuQuadColor, tempFloat, w, height + slctFrameSize,
            height + txtSelectionMedium.height + outerFrame + textToQuadBorder);
        render(menuQuads);

        // Name & >
        if(drawer.isFavorites)
        {
            tempString = "FAVORITES";
        }
        else
        {
            tempString = assets.getPackData(infos[i].id).name;
            uppercasify(tempString);
        }

        txtSelectionMedium.font.setString(tempString);
        tempFloat =
            std::max(txtIndent - getGlobalWidth(txtSelectionMedium.font) / 2.f,
                quadsIndent + arrowWidth + 2.f * slctFrameSize + outerFrame) +
            rightSideOffset;

        txtSelectionMedium.font.setPosition({tempFloat,
            height + textToQuadBorder - txtSelectionMedium.height * 0.8f});
        render(txtSelectionMedium.font);

        menuQuads.clear();
        menuQuads.reserve(8);

        if(i == drawer.packIdx)
        {
            // Draw > pointing downward, coordinates look a bit complicated
            // cause it's aligned with the middle point of the regular > arrows
            // The arrow is (packLabelHeight / 2.f - textToQuadBorder + 2.f *
            // slctFrameSize) wide
            height +=
                (packLabelHeight - textToQuadBorder - slctFrameSize) / 2.f;
            tempFloat = quadsIndent + arrowWidth / 2.f + slctFrameSize +
                        rightSideOffset;

            topLeft = {tempFloat - arrowWidth, height};
            bottomLeft = {tempFloat - arrowWidth, height + 2.f * slctFrameSize};

            height += arrowWidth;

            topRight = {tempFloat, height};
            bottomRight = {tempFloat, height + 2.f * slctFrameSize};

            menuQuads.batch_unsafe_emplace_back(
                menuTextColor, topLeft, bottomLeft, bottomRight, topRight);

            topLeft = {tempFloat, height};
            bottomLeft = {tempFloat, height + 2.f * slctFrameSize};

            height -= arrowWidth;
            tempFloat += arrowWidth;

            topRight = {tempFloat, height};
            bottomRight = {tempFloat, height + 2.f * slctFrameSize};

            menuQuads.batch_unsafe_emplace_back(
                menuTextColor, topLeft, bottomLeft, bottomRight, topRight);

            render(menuQuads);
        }
        else
        {
            height += slctFrameSize / 2.f;
            tempFloat = quadsIndent + rightSideOffset;

            topLeft = {tempFloat, height + textToQuadBorder};
            topRight = {
                tempFloat + 2.f * slctFrameSize, height + textToQuadBorder};

            height += packLabelHeight / 2.f;
            tempFloat += packLabelHeight / 2.f - textToQuadBorder;

            bottomLeft = {tempFloat, height};
            bottomRight = {tempFloat + 2.f * slctFrameSize, height};

            menuQuads.batch_unsafe_emplace_back(
                menuTextColor, topLeft, topRight, bottomRight, bottomLeft);

            topLeft = {tempFloat, height};
            topRight = {tempFloat + 2.f * slctFrameSize, height};

            height += packLabelHeight / 2.f;
            tempFloat = quadsIndent + rightSideOffset;

            bottomLeft = {tempFloat, height - textToQuadBorder};
            bottomRight = {
                tempFloat + 2.f * slctFrameSize, height - textToQuadBorder};

            menuQuads.batch_unsafe_emplace_back(
                menuTextColor, topLeft, topRight, bottomRight, bottomLeft);

            render(menuQuads);
            height -= slctFrameSize / 2.f;
        }

        i = ssvu::getMod(i + 1, packsSize);
        if(i == 0)
        {
            height = drawer.YOffset;
        }
    }
    while(i != ssvu::getMod(drawer.packIdx + 1, packsSize));
}

void MenuGame::drawLevelSelectionLeftSide(
    LevelDrawer& drawer, const bool revertOffset)
{
    constexpr float lineThickness = 2.f;
    const PackData& curPack{
        assets.getPackData(assets.getPackInfos()[drawer.packIdx].id)};
    const LevelData& levelData{
        assets.getLevelData(drawer.levelDataIds.at(drawer.currentIndex))};

    const float sidepanelIndent{w * 0.33f}, quadsIndent{w - sidepanelIndent},
        leftSideOffset{
            calcMenuOffset(levelDetailsOffset, w - quadsIndent, revertOffset)},
        smallInterline{txtSelectionSmall.height * 1.5f},
        smallLeftInterline{txtSelectionLSmall.height * 1.5f},
        postTitleSpace{txtSelectionMedium.height +
                       (smallLeftInterline - txtSelectionLSmall.height) -
                       txtSelectionLSmall.height * fontTopBorder},
        preLineSpace{txtSelectionMedium.height / 2.f +
                     txtSelectionLSmall.height * (1.f + fontTopBorder)},
        textXPos{textToQuadBorder - leftSideOffset},
        textRightBorder{getMaximumTextWidth()};
    float width{sidepanelIndent - leftSideOffset}, height{textToQuadBorder},
        tempFloat;

    //-------------------------------------
    // Backdrop - Right border

    menuQuads.clear();
    menuQuads.reserve(8);
    createQuad({menuTextColor.r, menuTextColor.g, menuTextColor.b, 150}, 0,
        width, 0, h);
    createQuad(menuQuadColor, width, width + lineThickness, 0, h);
    render(menuQuads);
    menuQuads.clear();

    //-------------------------------------
    // Level name

    std::string tempString{levelData.name};
    scrollNameRightBorder(tempString, txtSelectionBig.font,
        namesScroll[static_cast<int>(Label::LevelName)], textRightBorder);
    renderText(tempString, txtSelectionBig.font,
        {textXPos, height - txtSelectionBig.height * fontTopBorder});

    //-------------------------------------
    // Level description
    height += txtSelectionBig.height + textToQuadBorder -
              txtSelectionSmall.height * 0.7f;

    int i;
    for(i = 0; i < static_cast<int>(levelDescription.size()); ++i)
    {
        renderText(
            levelDescription[i], txtSelectionSmall.font, {textXPos, height});
        height +=
            i == descLines - 1 ? txtSelectionSmall.height : smallInterline;
    }
    if(i != descLines)
    {
        height += smallInterline * std::max(0, descLines - 1 - i) +
                  txtSelectionSmall.height;
    }

    height += textToQuadBorder + txtSelectionSmall.height * 0.7f;

    //-------------------------------------
    // Difficulty

    menuQuads.reserve_more(8);

    // Top line
    height += lineThickness;
    createQuad(menuQuadColor, 0, width, height - lineThickness, height);

    // Text
    height += 0.5f * txtSelectionMedium.height;

    renderText("DIFFICULTY: ", txtSelectionMedium.font,
        {textXPos, height - txtSelectionMedium.height}, menuQuadColor);

    tempString =
        diffMults.size() > 1
            ? "< " + toStr(ssvu::getByModIdx(diffMults, diffMultIdx)) + " >"
            : "NONE";

    const float difficultyBumpFactor =
        1.f + ((difficultyBumpEffect / difficultyBumpEffectMax) * 0.25f);
    txtSelectionMedium.font.setScale(
        difficultyBumpFactor, difficultyBumpFactor);

    renderText(tempString, txtSelectionMedium.font,
        {textXPos + txtSelectionMedium.font.getGlobalBounds().width,
            height - txtSelectionMedium.height});

    txtSelectionMedium.font.setScale(1.f, 1.f);

    // Bottom line
    height += txtSelectionMedium.height * 1.5f;
    createQuad(menuQuadColor, 0, width, height, height + lineThickness);
    height += lineThickness;

    //-------------------------------------
    // Pack info

    // "PACK"
    height += txtSelectionMedium.height / 2.f;
    renderText("PACK", txtSelectionMedium.font,
        {textToQuadBorder - leftSideOffset,
            height - txtSelectionMedium.height});

    // Pack name
    height += postTitleSpace;
    tempString = curPack.name;
    scrollNameRightBorder(tempString, "NAME: ", txtSelectionLSmall.font,
        namesScroll[static_cast<int>(Label::PackName)], textRightBorder);
    renderText(tempString, txtSelectionLSmall.font, {textXPos, height});

    // Pack author
    height += smallLeftInterline;
    tempString = curPack.author;
    scrollNameRightBorder(tempString, "AUTHOR: ", txtSelectionLSmall.font,
        namesScroll[static_cast<int>(Label::PackAuthor)], textRightBorder);
    renderText(tempString, txtSelectionLSmall.font, {textXPos, height});

    // Version
    height += smallLeftInterline;
    tempString = "VERSION: " + toStr(curPack.version);
    Utils::uppercasify(tempString);
    renderText(tempString, txtSelectionLSmall.font, {textXPos, height});

    // Bottom line
    menuQuads.reserve_more(4);
    height += preLineSpace;
    createQuad(menuQuadColor, 0, width, height, height + lineThickness);
    height += lineThickness;

    //-------------------------------------
    // Music info

    // "MUSIC"
    height += txtSelectionMedium.height / 2.f;
    renderText("MUSIC", txtSelectionMedium.font,
        {textToQuadBorder - leftSideOffset,
            height - txtSelectionMedium.height});

    // Track name
    const MusicData& musicDataTemp =
        assets.getMusicData(levelData.packId, levelData.musicId);
    height += postTitleSpace;
    tempString = musicDataTemp.name;
    scrollNameRightBorder(tempString, "NAME: ", txtSelectionLSmall.font,
        namesScroll[static_cast<int>(Label::MusicName)], textRightBorder);
    renderText(tempString, txtSelectionLSmall.font, {textXPos, height});

    // Track author
    height += smallLeftInterline;
    tempString = musicDataTemp.author;
    scrollNameRightBorder(tempString, "AUTHOR: ", txtSelectionLSmall.font,
        namesScroll[static_cast<int>(Label::MusicAuthor)], textRightBorder);
    renderText(tempString, txtSelectionLSmall.font, {textXPos, height});

    // Album name
    height += smallLeftInterline;
    tempString = !musicDataTemp.album.empty() ? musicDataTemp.album : "NONE";
    scrollNameRightBorder(tempString, "ALBUM: ", txtSelectionLSmall.font,
        namesScroll[static_cast<int>(Label::MusicAlbum)], textRightBorder);
    renderText(tempString, txtSelectionLSmall.font, {textXPos, height});

    //-------------------------------------
    // Favorite "button"

    height += preLineSpace;
    tempFloat = height + 3.f * txtSelectionMedium.height;
    menuQuads.reserve_more(8 * 5);

    // Frame
    createQuad(menuQuadColor, lineThickness - leftSideOffset, width, height,
        height + lineThickness);
    createQuad(menuQuadColor, -leftSideOffset, lineThickness - leftSideOffset,
        height, tempFloat);
    createQuad(menuQuadColor, width, width, height, tempFloat);
    createQuad(menuQuadColor, lineThickness - leftSideOffset, width,
        tempFloat - lineThickness, tempFloat);
    // Backdrop
    createQuad(menuSelectionColor, lineThickness - leftSideOffset, width,
        height + lineThickness, tempFloat - lineThickness);

    // Also renders all previous quads
    render(menuQuads);
    menuQuads.clear();

    renderTextCenteredOffset(
        levelData.favorite ? "UNFAVORITE - F1" : "FAVORITE - F1",
        txtSelectionMedium.font, {sidepanelIndent / 2.f, height},
        -leftSideOffset, menuQuadColor);

    height = tempFloat;

    //-------------------------------------
    // Leaderboards

    // "LEADERBOARDS"
    height += textToQuadBorder;
    renderTextCenteredOffset("LEADERBOARDS", txtSelectionBig.font,
        {sidepanelIndent / 2.f,
            height - txtSelectionBig.height * fontTopBorder},
        -leftSideOffset);

    // Line
    height += txtSelectionBig.height + textToQuadBorder;
    menuQuads.reserve(4);
    createQuad(menuQuadColor, 0, width, height, height + lineThickness);
    height += lineThickness;

    // Personal best
    height += txtSelectionMedium.height / 2.f;
    renderText("PERSONAL BEST", txtSelectionMedium.font,
        {textToQuadBorder - leftSideOffset,
            height - txtSelectionMedium.height * fontTopBorder},
        menuQuadColor);

    height += txtSelectionMedium.height + txtSelectionSmall.height;
    tempString = getLocalValidator(
        levelData.id, ssvu::getByModIdx(diffMults, diffMultIdx));
    renderText(toStr(assets.getCurrentLocalProfile().getScore(tempString)),
        txtSelectionScore.font,
        {textToQuadBorder - leftSideOffset,
            height - txtSelectionScore.height * fontTopBorder});

    // Line
    height += txtSelectionScore.height + txtSelectionMedium.height / 2.f;
    menuQuads.reserve_more(4);
    createQuad(menuQuadColor, 0, width, height, height + lineThickness);
    height += lineThickness;

    // TODO: uncomment when online is available
    /*

    // "GLOBAL"
    height += txtSmallHeight / 2.f;

    renderTextCenteredOffset("<< GLOBAL >>", txtSelectionMedium.font,
        {sidepanelIndent / 2.f, height - txtMediumHeight}, -panelOffset);

    // Line
    height += txtSmallHeight / 2.f + txtMediumHeight;
    menuQuads.reserve_more(4);
    createQuad(menuQuadColor, 0, width, height, height + lineThickness);
    height += lineThickness;

    // "USERNAME" and "TIME"
    tempFloat = sidepanelIndent * 0.6f;
    menuQuads.reserve_more(4);
    createQuad(menuQuadColor, tempFloat - lineThickness / 2.f - panelOffset,
        tempFloat + lineThickness / 2.f - panelOffset, height, h);

    height += txtSmallHeight / 2.f;
    renderTextCenteredOffset("USERNAME", txtSelectionMedium.font,
        {tempFloat / 2.f, height - txtMediumHeight}, -panelOffset);

    renderTextCenteredOffset("TIME", txtSelectionMedium.font,
        {tempFloat + sidepanelIndent * 0.2f, height - txtMediumHeight},
        -panelOffset);

    // Line
    height += txtSmallHeight / 2.f + txtMediumHeight;
    menuQuads.reserve_more(8);
    createQuad(menuQuadColor, 0, tempFloat - lineThickness / 2.f - panelOffset,
        height, height + lineThickness);
    createQuad(menuQuadColor, tempFloat + lineThickness / 2.f - panelOffset,
        width, height, height + lineThickness);
    height += lineThickness;

    // When online will be re-enabled there will be a list of the global
    // leaderboards here
    height += (h - height) / 2.f;
    renderTextCenteredOffset("ONLINE DISABLED", txtSelectionLSmall,
        {tempFloat / 2.f, height - txtMediumHeight * 1.5f}, -panelOffset);
    */

    render(menuQuads);
}

void MenuGame::drawGraphics()
{
    render(titleBar);
    render(creditsBar1);
    render(creditsBar2);
    render(txtVersion.font);
}

void MenuGame::draw()
{
    styleData.computeColors(levelStatus);
    window.clear(Color{0, 0, 0, 255});

    backgroundCamera.apply();
    const bool mainOrAbove{state >= States::SMain};
    if(mainOrAbove)
    {
        styleData.drawBackgroundMenu(
            window, ssvs::zeroVec2f, levelStatus, fourByThree);
    }
    overlayCamera.apply();
    if(mainOrAbove && state != States::LevelSelection && Config::getOnline())
    {
        renderText("CURRENT PROFILE: " + assets.pGetName(),
            txtSelectionLSmall.font,
            sf::Vector2f{20.f, getGlobalBottom(titleBar) + 8});
    }

    float indentBig{400.f}, indentSmall{540.f}, profileIndent{-100.f};
    // We need different values to fit menus in 4:3
    if(fourByThree)
    {
        indentBig = 280.f;
        indentSmall = 410.f;
        profileIndent = -75.f;
    }

    switch(state)
    {
        case States::LoadingScreen:
        {
            drawLoadResults();
            renderText("PRESS ANY KEY OR BUTTON TO CONTINUE", txtProf.font,
                {txtProf.height, h - txtProf.height * 2.7f});
        }
            return;

        case States::EpilepsyWarning:
        {
            render(epilepsyWarning);
            renderText("PRESS ANY KEY OR BUTTON TO CONTINUE", txtProf.font,
                {txtProf.height, h - txtProf.height * 2.7f});
        }
            return;

        case States::ETLPNewBoot:
            drawEnteringTextBoot();
            drawGraphics();
            break;

        case States::SLPSelectBoot:
            drawProfileSelectionBoot();
            drawGraphics();
            break;

        case States::SMain:
            // Fold previous menus
            if(optionsMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(optionsMenu.getCategoryByName("options"),
                    w - indentBig, true);
            }
            if(profileSelectionMenu.getCategory().getOffset() != 0.f)
            {
                drawProfileSelection(profileIndent, true);
            }
            if(enteringTextOffset != 0.f)
            {
                drawEnteringText(profileIndent, true);
            }
            if(levelDetailsOffset != 0.f)
            {
                drawLevelSelectionRightSide(*lvlDrawer, true);
                drawLevelSelectionLeftSide(*lvlDrawer, true);
            }

            // Draw main menu (right)
            drawMainSubmenus(mainMenu.getCategories(), indentBig);
            drawGraphics();
            break;

        case States::MOpts:
            // fold the main menu
            if(mainMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(
                    mainMenu.getCategoryByName("main"), w - indentBig, true);
            }

            // Option Menu (right)
            drawMainMenu(
                optionsMenu.getCategoryByName("options"), w - indentBig, false);

            // Draw options submenus (left)
            drawSubmenusSmall(optionsMenu.getCategories(), indentSmall);
            drawGraphics();
            break;

        case States::ETLPNew:
            drawMainMenu(mainMenu.getCategoryByName("local profiles"),
                w - indentBig, false);
            drawEnteringText(profileIndent, false);
            drawGraphics();
            break;

        case States::SLPSelect:
            drawMainMenu(mainMenu.getCategoryByName("local profiles"),
                w - indentBig, false);
            drawProfileSelection(profileIndent, false);
            drawGraphics();
            break;

        case States::LevelSelection:
            // fold the main menu
            if(mainMenu.getCategory().getOffset() != 0.f)
            {
                drawMainMenu(
                    mainMenu.getCategoryByName("main"), w - indentBig, true);
            }

            if(isFavoriteLevels())
            {
                drawLevelSelectionRightSide(favSlct, false);
                if(lvlSlct.XOffset != 0.f)
                {
                    drawLevelSelectionRightSide(lvlSlct, true);
                }
            }
            else
            {
                drawLevelSelectionRightSide(lvlSlct, false);
                if(favSlct.XOffset != 0.f)
                {
                    drawLevelSelectionRightSide(favSlct, true);
                }
            }
            drawLevelSelectionLeftSide(*lvlDrawer, false);
            break;

        case States::MWlcm: drawWelcome(); break;

        default: break;
    }

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
        dialogBox.draw(dialogBoxTextColor, styleData.getColor(0));
    }
}

void MenuGame::drawWelcome()
{
    // drawMenu(welcomeMenu);

    /*
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
     */
}

} // namespace hg

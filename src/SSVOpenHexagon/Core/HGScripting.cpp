// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SFML/Base/Fmt/Fmt.hpp"

#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Core/CustomTimeline.hpp"
#include "SSVOpenHexagon/Core/CustomTimelineHandle.hpp"
#include "SSVOpenHexagon/Core/CustomTimelineManager.hpp"
#include "SSVOpenHexagon/Core/Frametime.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/LuaScripting.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadataProxy.hpp"
#include "SSVOpenHexagon/Utils/Timeline2.hpp"
#include "SSVOpenHexagon/Utils/TypeWrapper.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/IO.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/Decay.hpp"

#include <stdexcept>

namespace hg
{

template <typename F>
Utils::LuaMetadataProxy addLuaFn(Lua::LuaContext& lua, const sf::base::String& name, F&& f)
{
    // TODO (P2): reduce instantiations by using captureless lambdas
    lua.writeVariable(name.cStr(), SFML_BASE_FORWARD(f));
    return Utils::LuaMetadataProxy{Utils::TypeWrapper<F>{}, LuaScripting::getMetadata(), name};
}

void HexagonGame::initLua_Utils()
{
    // ------------------------------------------------------------------------
    // Used internally to track values in the console.
    lua.writeVariable("u_impl_addTrackedResult",
                      [this](const sf::base::String& result) { ilcLuaTrackedResults.emplaceBack(result); });

    // ------------------------------------------------------------------------
    addLuaFn(lua,
             "u_setFlashEffect", //
             [this](float mIntensity) { status.flashEffect = mIntensity; })
        .arg("value")
        .doc("Flash the screen with `$0` intensity (from 0 to 255).");

    addLuaFn(lua,
             "u_setFlashColor", //
             [this](int r, int g, int b) { initFlashEffect(r, g, b); })
        .arg("r")
        .arg("g")
        .arg("b")
        .doc("Set the color of the flash effect to `{$0, $1, $2}`.");
    // ------------------------------------------------------------------------

    addLuaFn(lua,
             "u_log", //
             [this](const sf::base::String& mLog)
    {
        if (window == nullptr) // headless
        {
            return;
        }

        hg::lo("lua") << mLog << '\n';
        ilcCmdLog.emplaceBack("[lua]: " + mLog + '\n');
    })
        .arg("message")
        .doc("Print out `$0` to the console.");

    addLuaFn(lua,
             "u_haltTime", //
             [this](double mDuration) { status.pauseTime(getFTToSeconds(mDuration)); })
        .arg("duration")
        .doc("Pause the game timer for `$0` seconds.");

    addLuaFn(lua,
             "u_clearWalls", //
             [this]
    {
        walls.clear();
    }).doc("Remove all existing walls.");

    addLuaFn(lua,
             "u_getPlayerAngle", //
             [this]
    {
        return player.getPlayerAngle();
    }).doc("Return the current angle of the player, in radians.");

    addLuaFn(lua, "u_setPlayerAngle", [this](float newAng) { player.setPlayerAngle(newAng); })
        .arg("angle")
        .doc("Set the current angle of the player to `$0`, in radians.");

    addLuaFn(lua,
             "u_isFastSpinning", //
             [this] { return status.fastSpin > 0; })
        .doc(
            "Return `true` if the camera is currently \"fast spinning\", "
            "`false` otherwise.");

    addLuaFn(lua,
             "u_forceIncrement", //
             [this] { incrementDifficulty(); })
        .doc(
            "Immediately force a difficulty increment, regardless of the "
            "chosen automatic increment parameters.");

    addLuaFn(lua,
             "u_getDifficultyMult", //
             [this]
    {
        return difficultyMult;
    }).doc("Return the current difficulty multiplier.");

    addLuaFn(lua,
             "u_getSpeedMultDM", //
             [this] { return getSpeedMultDM(); })
        .doc(
            "Return the current speed multiplier, adjusted for the chosen "
            "difficulty multiplier.");

    addLuaFn(lua,
             "u_getDelayMultDM", //
             [this] { return getDelayMultDM(); })
        .doc(
            "Return the current delay multiplier, adjusted for the chosen "
            "difficulty multiplier.");

    addLuaFn(lua,
             "u_swapPlayer", //
             [this](bool mPlaySound) { performPlayerSwap(mPlaySound); })
        .arg("playSound")
        .doc(
            "Force-swaps (180 degrees) the player when invoked. If `$0` is "
            "`true`, the swap sound will be played.");
}

void HexagonGame::initLua_AudioControl()
{
    addLuaFn(lua,
             "a_setMusic", //
             [this](const sf::base::String& mId)
    {
        musicData           = assets.getMusicData(levelData->packId, mId);
        musicData.firstPlay = true;
        stopLevelMusic();
        playLevelMusic();
    })
        .arg("musicId")
        .doc(
            "Stop the current music and play the music with id `$0`. The id is "
            "defined in the music `.json` file, under `\"id\"`.");

    addLuaFn(lua,
             "a_setMusicSegment", //
             [this](const sf::base::String& mId, int segment)
    {
        musicData = assets.getMusicData(levelData->packId, mId);
        stopLevelMusic();
        playLevelMusicAtTime(musicData.getSegment(segment).time);
    })
        .arg("musicId")
        .arg("segment")
        .doc(
            "Stop the current music and play the music with id `$0`, starting "
            "at segment `$1`. Segments are defined in the music `.json` file, "
            "under `\"segments\"`.");

    addLuaFn(lua,
             "a_setMusicSeconds", //
             [this](const sf::base::String& mId, float mTime)
    {
        musicData = assets.getMusicData(levelData->packId, mId);
        stopLevelMusic();
        playLevelMusicAtTime(mTime);
    })
        .arg("musicId")
        .arg("time")
        .doc(
            "Stop the current music and play the music with id `$0`, starting "
            "at time `$1` (in seconds).");

    addLuaFn(lua,
             "a_playSound", //
             [this](const sf::base::String& mId) { playSoundOverride(mId); })
        .arg("soundId")
        .doc(
            "Play the sound with id `$0`. The id must be registered in "
            "`assets.json`, under `\"soundBuffers\"`.");

    addLuaFn(lua,
             "a_playPackSound", //
             [this](const sf::base::String& fileName) { playPackSoundOverride(getPackId(), fileName); })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "plays the specified file `$0`.");

    addLuaFn(lua,
             "a_syncMusicToDM", //
             [this](bool value)
    {
        levelStatus.syncMusicToDM = value;
        refreshMusicPitch();
    })
        .arg("value")
        .doc(
            "This function, when called, overrides the user's preference of "
            "adjusting the music's pitch to the difficulty multiplier. Useful "
            "for levels that rely on the music to time events.");

    addLuaFn(lua,
             "a_setMusicPitch", //
             [this](float mPitch)
    {
        levelStatus.musicPitch = mPitch;
        refreshMusicPitch();
    })
        .arg("pitch")
        .doc(
            "Manually adjusts the pitch of the music by multiplying it by "
            "`$0`. The amount the pitch shifts may change on DM multiplication "
            "and user's preference of the music pitch. **Negative values will "
            "not work!**");

    addLuaFn(lua,
             "a_overrideBeepSound", //
             [this](const sf::base::String& mId) { levelStatus.beepSound = qualifyPackAsset(mId); })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new beep sound. This only "
            "applies to the particular level where this function is called.");

    addLuaFn(lua,
             "a_overrideIncrementSound", //
             [this](const sf::base::String& mId) { levelStatus.levelUpSound = qualifyPackAsset(mId); })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new increment sound. This "
            "only "
            "applies to the particular level where this function is called.");

    addLuaFn(lua,
             "a_overrideSwapSound", //
             [this](const sf::base::String& mId) { levelStatus.swapSound = qualifyPackAsset(mId); })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new swap sound. This only "
            "applies to the particular level where this function is called.");

    addLuaFn(lua,
             "a_overrideDeathSound", //
             [this](const sf::base::String& mId) { levelStatus.deathSound = qualifyPackAsset(mId); })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new death sound. This only "
            "applies to the particular level where this function is called.");
}

static void waitUntilSImpl(const double mDuration, const HexagonGameStatus& status, Utils::timeline2& timeline)
{
    timeline.append_wait_until_fn([&status, mDuration]
    { return status.getLevelStartTP() + std::chrono::milliseconds(static_cast<int>(mDuration * 1000.0)); });
}

void HexagonGame::initLua_MainTimeline()
{
    addLuaFn(lua,
             "t_eval",
             [this](const sf::base::String& mCode)
    { timeline.append_do([this, mCode] { Utils::runLuaCode(lua, mCode); }); })
        .arg("code")
        .doc(
            "*Add to the main timeline*: evaluate the Lua code specified in "
            "`$0`.");

    addLuaFn(lua, "t_clear", [this]() { timeline.clear(); }).doc("Clear the main timeline.");

    addLuaFn(lua,
             "t_kill", //
             [this]
    {
        timeline.append_do([this] { death(true); });
    }).doc("*Add to the main timeline*: kill the player.");

    addLuaFn(lua, "t_wait", [this](double mDuration) { timeline.append_wait_for_sixths(mDuration); })
        .arg("duration")
        .doc(
            "*Add to the main timeline*: wait for `$0` frames (under the "
            "assumption of a 60 FPS frame rate).");

    addLuaFn(lua,
             "t_waitS", //
             [this](double mDuration) { timeline.append_wait_for_seconds(mDuration); })
        .arg("duration")
        .doc("*Add to the main timeline*: wait for `$0` seconds.");

    addLuaFn(lua,
             "t_waitUntilS", //
             [this](double mDuration) { waitUntilSImpl(mDuration, status, timeline); })
        .arg("duration")
        .doc(
            "*Add to the main timeline*: wait until the timer reaches `$0` "
            "seconds.");
}

void HexagonGame::initLua_EventTimeline()
{
    addLuaFn(lua,
             "e_eval",
             [this](const sf::base::String& mCode)
    { eventTimeline.append_do([=, this] { Utils::runLuaCode(lua, mCode); }); })
        .arg("code")
        .doc(
            "*Add to the event timeline*: evaluate the Lua code specified in "
            "`$0`. (This is the closest you'll get to 1.92 events)");

    addLuaFn(lua,
             "e_kill", //
             [this]
    {
        eventTimeline.append_do([this] { death(true); });
    }).doc("*Add to the event timeline*: kill the player.");

    addLuaFn(lua,
             "e_stopTime", //
             [this](double mDuration)
    { eventTimeline.append_do([this, mDuration] { status.pauseTime(getFTToSeconds(mDuration)); }); })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` frames "
            "(under the assumption of a 60 FPS frame rate).");

    addLuaFn(lua,
             "e_stopTimeS", //
             [this](double mDuration) { eventTimeline.append_do([this, mDuration] { status.pauseTime(mDuration); }); })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` "
            "seconds.");

    addLuaFn(lua, "e_wait", [this](double mDuration) { eventTimeline.append_wait_for_sixths(mDuration); })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait for `$0` frames (under the "
            "assumption of a 60 FPS frame rate).");

    addLuaFn(lua,
             "e_waitS", //
             [this](double mDuration) { eventTimeline.append_wait_for_seconds(mDuration); })
        .arg("duration")
        .doc("*Add to the event timeline*: wait for `$0` seconds.");

    addLuaFn(lua,
             "e_waitUntilS", //
             [this](double mDuration) { waitUntilSImpl(mDuration, status, eventTimeline); })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait until the timer reaches `$0` "
            "seconds.");

    addLuaFn(lua,
             "e_messageAdd", //
             [this](const sf::base::String& mMsg, double mDuration)
    {
        eventTimeline.append_do([this, mMsg, mDuration]
        {
            if (firstPlay)
            {
                addMessage(mMsg, mDuration, /* mSoundToggle */ true);
            }
        });
    })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will only be printed during the first "
            "run of the level.");

    addLuaFn(lua,
             "e_messageAddImportant", //
             [this](const sf::base::String& mMsg, double mDuration)
    { eventTimeline.append_do([this, mMsg, mDuration] { addMessage(mMsg, mDuration, /* mSoundToggle */ true); }); })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will be printed during every run of the "
            "level.");

    addLuaFn(lua,
             "e_messageAddImportantSilent",
             [this](const sf::base::String& mMsg, double mDuration)
    { eventTimeline.append_do([this, mMsg, mDuration] { addMessage(mMsg, mDuration, /* mSoundToggle */ false); }); })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will only be printed during every "
            "run of the level, and will not produce any sound.");

    addLuaFn(lua,
             "e_clearMessages", //
             [this]
    {
        clearMessages();
    }).doc("Remove all previously scheduled messages.");
}

void HexagonGame::initLua_CustomTimelines()
{
    addLuaFn(lua,
             "ct_create", //
             [this]() -> CustomTimelineHandle { return _customTimelineManager.create(); })
        .doc(
            "Create a new custom timeline and return a integer handle "
            "to it.");


    const auto checkHandle = [this](CustomTimelineHandle cth, const char* title) -> bool
    {
        if (_customTimelineManager.isHandleValid(cth))
        {
            return true;
        }

        hg::lo("CustomTimelineManager") << "Invalid handle '" << cth << "' during '" << title << "'\n";

        return false;
    };

    addLuaFn(lua,
             "ct_eval",
             [checkHandle, this](CustomTimelineHandle cth, const sf::base::String& mCode)
    {
        if (!checkHandle(cth, "ct_eval"))
        {
            return;
        }

        _customTimelineManager.get(cth)._timeline.append_do([this, mCode] { Utils::runLuaCode(lua, mCode); });
    })
        .arg("handle")
        .arg("code")
        .doc(
            "*Add to the custom timeline with handle `$0`*: evaluate the Lua "
            "code specified in `$1`.");

    addLuaFn(lua,
             "ct_kill", //
             [checkHandle, this](CustomTimelineHandle cth)
    {
        if (!checkHandle(cth, "ct_kill"))
        {
            return;
        }

        _customTimelineManager.get(cth)._timeline.append_do([this] { death(true); });
    })
        .arg("handle")
        .doc("*Add to the custom timeline with handle `$0`*: kill the player.");

    addLuaFn(lua,
             "ct_stopTime", //
             [checkHandle, this](CustomTimelineHandle cth, double mDuration)
    {
        if (!checkHandle(cth, "ct_stopTime"))
        {
            return;
        }

        _customTimelineManager.get(cth)._timeline.append_do([this, mDuration]
        { status.pauseTime(getFTToSeconds(mDuration)); });
    })
        .arg("handle")
        .arg("duration")
        .doc(
            "*Add to the custom timeline with handle `$0`*: pause the game "
            "timer for `$1` frames (under the assumption of a 60 FPS frame "
            "rate).");

    addLuaFn(lua,
             "ct_stopTimeS", //
             [checkHandle, this](CustomTimelineHandle cth, double mDuration)
    {
        if (!checkHandle(cth, "ct_stopTimeS"))
        {
            return;
        }

        _customTimelineManager.get(cth)._timeline.append_do([this, mDuration] { status.pauseTime(mDuration); });
    })
        .arg("handle")
        .arg("duration")
        .doc(
            "*Add to the custom timeline with handle `$0`*: pause the game "
            "timer for `$1` seconds.");

    addLuaFn(lua,
             "ct_wait",
             [checkHandle, this](CustomTimelineHandle cth, double mDuration)
    {
        if (!checkHandle(cth, "ct_wait"))
        {
            return;
        }

        _customTimelineManager.get(cth)._timeline.append_wait_for_sixths(mDuration);
    })
        .arg("handle")
        .arg("duration")
        .doc(
            "*Add to the custom timeline with handle `$0`*: wait for `$1` "
            "frames (under the assumption of a 60 FPS frame rate).");

    addLuaFn(lua,
             "ct_waitS", //
             [checkHandle, this](CustomTimelineHandle cth, double mDuration)
    {
        if (!checkHandle(cth, "ct_waitS"))
        {
            return;
        }

        _customTimelineManager.get(cth)._timeline.append_wait_for_seconds(mDuration);
    })
        .arg("handle")
        .arg("duration")
        .doc(
            "*Add to the custom timeline with handle `$0`*: wait for `$1` "
            "seconds.");

    addLuaFn(lua,
             "ct_waitUntilS", //
             [checkHandle, this](CustomTimelineHandle cth, double mDuration)
    {
        if (!checkHandle(cth, "ct_waitUntilS"))
        {
            return;
        }

        waitUntilSImpl(mDuration, status, _customTimelineManager.get(cth)._timeline);
    })
        .arg("handle")
        .arg("duration")
        .doc(
            "*Add to the custom timeline with handle `$0`*: wait until the "
            "timer reaches `$1` seconds.");
}

template <typename T>
auto HexagonGame::makeLuaAccessor(T& obj, const sf::base::String& prefix)
{
    return
        [this,
         &obj,
         prefix](const sf::base::String& name, auto pmd, const sf::base::String& getterDesc, const sf::base::String& setterDesc)
    {
        using Type = SFML_BASE_DECAY(decltype(obj.*pmd));

        const sf::base::String getterString = prefix + "_get" + name;
        const sf::base::String setterString = prefix + "_set" + name;

        addLuaFn(lua,
                 getterString, //
                 [pmd, &obj]() -> Type
        {
            return obj.*pmd;
        }).doc(getterDesc);

        addLuaFn(lua,
                 setterString, //
                 [pmd, &obj](Type mValue) { obj.*pmd = mValue; })
            .arg("value")
            .doc(setterDesc);
    };
}

void HexagonGame::initLua_LevelControl()
{
    addLuaFn(lua,
             "l_overrideScore", //
             [this](const sf::base::String& mVar)
    {
        try
        {
            levelStatus.scoreOverride   = mVar;
            levelStatus.scoreOverridden = true;
            // Make sure we're not passing in a string
            lua.executeCode(("if (type(" + mVar + sf::base::String(R"LUACODE( ) ~= "number") then
								error("Score override must be a number value")
								end
)LUACODE"))
                                .cStr());
        } catch (const std::runtime_error& mError)
        {
            sf::base::printLn("[l_overrideScore] Runtime error on overriding score with level \"{}\": \n{}",
                              levelData->name,
                              mError.what());
            if (!Config::getDebug())
            {
                goToMenu(false /* mSendScores */, true /* mError */);
            }
        };
    })
        .arg("variable")
        .doc(
            "Overrides the default scoring method and determines score based "
            "off the value of `$0`. This allows for custom scoring in levels. "
            "*Avoid using strings, otherwise scores won't sort properly. NOTE: "
            "Your variable must be global for this to work.*");

    addLuaFn(lua,
             "l_setRotation", //
             [this](float mValue)
    {
        // TODO (P2): might break replays if someone uses this to control
        // game logic
        if (backgroundCamera.hasValue())
        {
            backgroundCamera->rotation = sf::degrees(mValue);
        }
    })
        .arg("angle")
        .doc("Set the background camera rotation to `$0` degrees.");

    addLuaFn(lua,
             "l_getRotation", //
             [this]
    {
        // TODO (P2): might break replays if someone uses this to control
        // game logic
        return backgroundCamera.hasValue() ? backgroundCamera->rotation.asDegrees() : 0.f;
    }).doc("Return the background camera rotation, in degrees.");

    addLuaFn(lua,
             "l_getOfficial", //
             [] { return Config::getOfficial(); })
        .doc(
            "Return `true` if \"official mode\" is enabled, `false` "
            "otherwise.");
}

void HexagonGame::initLua_StyleControl()
{
    addLuaFn(lua,
             "s_setStyle", //
             [this](const sf::base::String& mId)
    {
        styleData = assets.getStyleData(levelData->packId, mId);
        styleData.computeColors();
    })
        .arg("styleId")
        .doc(
            "Set the currently active style to the style with id `$0`. Styles "
            "can be defined as `.json` files in the `<pack>/Styles/` folder.");
}

void HexagonGame::initLua_WallCreation()
{
    addLuaFn(lua,
             "w_wall", //
             [this](int mSide, float mThickness)
    {
        timeline.append_do([this, mSide, mThickness]
        { createWall(mSide, mThickness, SpeedData{getSpeedMultDM()}, SpeedData{} /* curve */, 0.f /* hueMod */); });
    })
        .arg("side")
        .arg("thickness")
        .doc(
            "Create a new wall at side `$0`, with thickness `$1`. The speed of "
            "the wall will be calculated by using the speed multiplier, "
            "adjusted for the current difficulty multiplier.");

    addLuaFn(lua,
             "w_wallAdj", //
             [this](int mSide, float mThickness, float mSpeedAdj)
    {
        timeline.append_do([this, mSide, mThickness, mSpeedAdj] {
            createWall(mSide, mThickness, SpeedData{mSpeedAdj * getSpeedMultDM()}, SpeedData{} /* curve */, 0.f /* hueMod */);
        });
    })
        .arg("side")
        .arg("thickness")
        .arg("speedMult")
        .doc(
            "Create a new wall at side `$0`, with thickness `$1`. The speed of "
            "the wall will be calculated by using the speed multiplier, "
            "adjusted for the current difficulty multiplier, and finally "
            "multiplied by `$2`.");

    addLuaFn(lua,
             "w_wallAcc", //
             [this](int mSide, float mThickness, float mSpeedAdj, float mAcceleration, float mMinSpeed, float mMaxSpeed)
    {
        timeline.append_do([this, mSide, mThickness, mSpeedAdj, mAcceleration, mMinSpeed, mMaxSpeed]
        {
            createWall(mSide,
                       mThickness,
                       SpeedData{mSpeedAdj * getSpeedMultDM(),
                                 mAcceleration / (SFML_BASE_MATH_POWF(difficultyMult, 0.65f)),
                                 mMinSpeed * getSpeedMultDM(),
                                 mMaxSpeed * getSpeedMultDM()},
                       SpeedData{} /* curve */,
                       0.f /* hueMod */);
        });
    })
        .arg("side")
        .arg("thickness")
        .arg("speedMult")
        .arg("acceleration")
        .arg("minSpeed")
        .arg("maxSpeed")
        .doc(
            "Create a new wall at side `$0`, with thickness `$1`. The speed of "
            "the wall will be calculated by using the speed multiplier, "
            "adjusted for the current difficulty multiplier, and finally "
            "multiplied by `$2`. The wall will have a speed acceleration value "
            "of `$3`. The minimum and maximum speed of the wall are bounded by "
            "`$4` and `$5`, adjusted  for the current difficulty multiplier.");

    addLuaFn(lua,
             "w_wallHModSpeedData", //
             [this](float mHMod, int mSide, float mThickness, float mSAdj, float mSAcc, float mSMin, float mSMax, bool mSPingPong)
    {
        timeline.append_do([this, mHMod, mSide, mThickness, mSAdj, mSAcc, mSMin, mSMax, mSPingPong]
        {
            createWall(mSide,
                       mThickness,
                       SpeedData{mSAdj * getSpeedMultDM(), mSAcc, mSMin, mSMax, mSPingPong},
                       SpeedData{} /* curve */,
                       mHMod);
        });
    })
        .arg("hueModifier")
        .arg("side")
        .arg("thickness")
        .arg("speedMult")
        .arg("acceleration")
        .arg("minSpeed")
        .arg("maxSpeed")
        .arg("pingPong")
        .doc(
            "Create a new wall at side `$1`, with thickness `$2`. The speed of "
            "the wall will be calculated by using the speed multiplier, "
            "adjusted for the current difficulty multiplier, and finally "
            "multiplied by `$3`. The wall will have a speed acceleration value "
            "of `$4`. The minimum and maximum speed of the wall are bounded by "
            "`$5` and `$6`, adjusted  for the current difficulty multiplier. "
            "The hue of the wall will be adjusted by `$0`. If `$7` is enabled, "
            "the wall will accelerate back and forth between its minimum and "
            "maximum speed.");

    addLuaFn(lua,
             "w_wallHModCurveData", //
             [this](float mHMod, int mSide, float mThickness, float mCAdj, float mCAcc, float mCMin, float mCMax, bool mCPingPong)
    {
        timeline.append_do([this, mHMod, mSide, mThickness, mCAdj, mCAcc, mCMin, mCMax, mCPingPong]
        {
            createWall(mSide, mThickness, SpeedData{getSpeedMultDM()}, SpeedData{mCAdj, mCAcc, mCMin, mCMax, mCPingPong}, mHMod);
        });
    })
        .arg("hueModifier")
        .arg("side")
        .arg("thickness")
        .arg("curveSpeedMult")
        .arg("curveAcceleration")
        .arg("curveMinSpeed")
        .arg("curveMaxSpeed")
        .arg("pingPong")
        .doc(
            "Create a new curving wall at side `$1`, with thickness `$2`. The "
            "curving speed of the wall will be calculated by using the speed "
            "multiplier, adjusted for the current difficulty multiplier, and "
            "finally multiplied by `$3`. The wall will have a curving speed "
            "acceleration value of `$4`. The minimum and maximum curving speed "
            "of the wall are bounded by `$5` and `$6`, adjusted  for the "
            "current difficulty multiplier. The hue of the wall will be "
            "adjusted by `$0`. If `$7` is enabled, the wall will accelerate "
            "back and forth between its minimum and maximum speed.");
}

void HexagonGame::initLua_Steam()
{
    addLuaFn(lua,
             "steam_unlockAchievement", //
             [this](const sf::base::String& mId)
    {
        if (inReplay())
        {
            // Do not unlock achievements while watching a replay.
            return;
        }

        if (steamManager != nullptr && Config::getOfficial())
        {
            steamManager->unlock_achievement(mId.cStr());
        }
    })
        .arg("achievementId")
        .doc("Unlock the Steam achievement with id `$0`.");
}

void HexagonGame::initLua()
{
    LuaScripting::init(lua,
                       *rng,
                       false /* inMenu */,
                       cwManager,
                       levelStatus,
                       status,
                       styleData,
                       assets,
                       [this](const sf::base::String& filename) -> void { runLuaFile(filename); },
                       execScriptPackPathContext,
                       [this]() -> const sf::Path& { return levelData->packPath; },
                       [this]() -> const PackData& { return getPackData(); },
                       (window == nullptr) /* headless */);

    initLua_Utils();
    initLua_AudioControl();
    initLua_MainTimeline();
    initLua_EventTimeline();
    initLua_CustomTimelines();
    initLua_LevelControl();
    initLua_StyleControl();
    initLua_WallCreation();
    initLua_Steam();
}

void HexagonGame::runLuaFile(const sf::base::String& mFileName)
try
{
    Utils::runLuaFile(lua, mFileName);
} catch (...)
{
    if (!Config::getDebug())
    {
        goToMenu(false /* mSendScores */, true /* mError */);
    }
}

void HexagonGame::initLuaAndPrintDocs()
{
    initLua();
    LuaScripting::printDocs();
}

void HexagonGame::luaExceptionLippincottHandler(sf::base::StringView mName)
try
{
    throw;
} catch (const std::runtime_error& mError)
{
    sf::base::printLn("[runLuaFunctionIfExists] Runtime error on \"{}\" with level \"{}\": \n{}",
                      mName,
                      levelData->name,
                      mError.what());

    if (!Config::getDebug())
    {
        goToMenu(false /* mSendScores */, true /* mError */);
    }
} catch (...)
{
    sf::base::printLn("[runLuaFunctionIfExists] Unknown runtime error on \"{}\" with level \"{}\": \n",
                      mName,
                      levelData->name);

    if (!Config::getDebug())
    {
        goToMenu(false /* mSendScores */, true /* mError */);
    }
}

} // namespace hg

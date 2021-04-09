// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Audio.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Core/LuaScripting.hpp"
#include "SSVOpenHexagon/Utils/TypeWrapper.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <algorithm>

namespace hg
{

template <typename F>
Utils::LuaMetadataProxy addLuaFn(
    Lua::LuaContext& lua, const std::string& name, F&& f)
{
    lua.writeVariable(name, std::forward<F>(f));
    return Utils::LuaMetadataProxy{
        Utils::TypeWrapper<F>{}, LuaScripting::getMetadata(), name};
}

void HexagonGame::initLua_Utils()
{
    // ------------------------------------------------------------------------
    // Used internally to track values in the console.
    lua.writeVariable(
        "u_impl_addTrackedResult", [this](const std::string& result) {
            ilcLuaTrackedResults.emplace_back(result);
        });

    // ------------------------------------------------------------------------
    addLuaFn(lua, "u_setFlashEffect", //
        [this](float mIntensity) { status.flashEffect = mIntensity; })
        .arg("value")
        .doc("Flash the screen with `$0` intensity (from 0 to 255).");

    addLuaFn(lua, "u_log", //
        [this](const std::string& mLog) {
            ssvu::lo("lua") << mLog << '\n';
            ilcCmdLog.emplace_back("[lua]: " + mLog + '\n');
        })
        .arg("message")
        .doc("Print out `$0` to the console.");

    addLuaFn(lua, "u_execScript", //
        [this](const std::string& mScriptName) {
            runLuaFile(
                Utils::getDependentScriptFilename(execScriptPackPathContext,
                    levelData->packPath.getStr(), mScriptName));
        })
        .arg("scriptFilename")
        .doc("Execute the script located at `<pack>/Scripts/$0`.");

    addLuaFn(lua, "u_execDependencyScript", //
        [this](const std::string& mPackDisambiguator,
            const std::string& mPackName, const std::string& mPackAuthor,
            const std::string& mScriptName) {
            Utils::withDependencyScriptFilename(
                [this](const std::string& filename) { runLuaFile(filename); },
                execScriptPackPathContext, assets, getPackData(),
                mPackDisambiguator, mPackName, mPackAuthor, mScriptName);
        })
        .arg("packDisambiguator")
        .arg("packName")
        .arg("packAuthor")
        .arg("scriptFilename")
        .doc(
            "Execute the script provided by the dependee pack with "
            "disambiguator `$0`, name `$1`, author `$2`, located at "
            "`<dependeePack>/Scripts/$3`.");

    addLuaFn(lua, "u_isKeyPressed",
        [this](int mKey) {
            // TODO: this is not saved in replays. Deprecate?

            return window != nullptr &&
                   window->getInputState()[ssvs::KKey(mKey)];
        })
        .arg("keyCode")
        .doc(
            "Return `true` if the keyboard key with code `$0` is being "
            "pressed, `false` otherwise. The key code must match the "
            "definition of the SFML `sf::Keyboard::Key` enumeration.");

    addLuaFn(lua, "u_haltTime", //
        [this](double mDuration) {
            status.pauseTime(ssvu::getFTToSeconds(mDuration));
        })
        .arg("duration")
        .doc("Pause the game timer for `$0` seconds.");

    addLuaFn(lua, "u_clearWalls", //
        [this] { walls.clear(); })
        .doc("Remove all existing walls.");

    addLuaFn(lua, "u_getPlayerAngle", //
        [this] { return player.getPlayerAngle(); })
        .doc("Return the current angle of the player, in radians.");

    addLuaFn(lua, "u_setPlayerAngle",
        [this](float newAng) { player.setPlayerAngle(newAng); })
        .arg("angle")
        .doc("Set the current angle of the player to `$0`, in radians.");

    addLuaFn(lua, "u_isMouseButtonPressed",
        [this](int mKey) {
            // TODO: this is not saved in replays. Deprecate?

            return window != nullptr &&
                   window->getInputState()[ssvs::MBtn(mKey)];
        })
        .arg("buttonCode")
        .doc(
            "Return `true` if the mouse button with code `$0` is being "
            "pressed, `false` otherwise. The button code must match the "
            "definition of the SFML `sf::Mouse::Button` enumeration.");

    addLuaFn(lua, "u_isFastSpinning", //
        [this] { return status.fastSpin > 0; })
        .doc(
            "Return `true` if the camera is currently \"fast spinning\", "
            "`false` otherwise.");

    addLuaFn(lua, "u_forceIncrement", //
        [this] { incrementDifficulty(); })
        .doc(
            "Immediately force a difficulty increment, regardless of the "
            "chosen automatic increment parameters.");

    addLuaFn(lua, "u_getDifficultyMult", //
        [this] { return difficultyMult; })
        .doc("Return the current difficulty multiplier.");

    addLuaFn(lua, "u_getSpeedMultDM", //
        [this] { return getSpeedMultDM(); })
        .doc(
            "Return the current speed multiplier, adjusted for the chosen "
            "difficulty multiplier.");

    addLuaFn(lua, "u_getDelayMultDM", //
        [this] { return getDelayMultDM(); })
        .doc(
            "Return the current delay multiplier, adjusted for the chosen "
            "difficulty multiplier.");

    addLuaFn(lua, "u_swapPlayer", //
        [this](bool mPlaySound) { performPlayerSwap(mPlaySound); })
        .arg("playSound")
        .doc(
            "Force-swaps (180 degrees) the player when invoked. If `$0` is "
            "`true`, the swap sound will be played.");
}

void HexagonGame::initLua_AudioControl()
{
    addLuaFn(lua, "a_setMusic", //
        [this](const std::string& mId) {
            musicData = assets.getMusicData(levelData->packId, mId);
            musicData.firstPlay = true;
            stopLevelMusic();
            playLevelMusic();
        })
        .arg("musicId")
        .doc(
            "Stop the current music and play the music with id `$0`. The id is "
            "defined in the music `.json` file, under `\"id\"`.");

    addLuaFn(lua, "a_setMusicSegment", //
        [this](const std::string& mId, int segment) {
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

    addLuaFn(lua, "a_setMusicSeconds", //
        [this](const std::string& mId, float mTime) {
            musicData = assets.getMusicData(levelData->packId, mId);
            stopLevelMusic();
            playLevelMusicAtTime(mTime);
        })
        .arg("musicId")
        .arg("time")
        .doc(
            "Stop the current music and play the music with id `$0`, starting "
            "at time `$1` (in seconds).");

    addLuaFn(lua, "a_playSound", //
        [this](const std::string& mId) { playSoundOverride(mId); })
        .arg("soundId")
        .doc(
            "Play the sound with id `$0`. The id must be registered in "
            "`assets.json`, under `\"soundBuffers\"`.");

    addLuaFn(lua, "a_playPackSound", //
        [this](const std::string& fileName) {
            playPackSoundOverride(getPackId(), fileName);
        })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "plays the specified file `$0`.");

    addLuaFn(lua, "a_syncMusicToDM", //
        [this](bool value) {
            levelStatus.syncMusicToDM = value;
            refreshMusicPitch();
        })
        .arg("value")
        .doc(
            "This function, when called, overrides the user's preference of "
            "adjusting the music's pitch to the difficulty multiplier. Useful "
            "for levels that rely on the music to time events.");

    addLuaFn(lua, "a_setMusicPitch", //
        [this](float mPitch) {
            levelStatus.musicPitch = mPitch;
            refreshMusicPitch();
        })
        .arg("pitch")
        .doc(
            "Manually adjusts the pitch of the music by multiplying it by "
            "`$0`. The amount the pitch shifts may change on DM multiplication "
            "and user's preference of the music pitch. **Negative values will "
            "not work!**");

    addLuaFn(lua, "a_overrideBeepSound", //
        [this](const std::string& mId) {
            levelStatus.beepSound = getPackId() + "_" + mId;
        })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new beep sound. This only "
            "applies to the particular level where this function is called.");

    addLuaFn(lua, "a_overrideIncrementSound", //
        [this](const std::string& mId) {
            levelStatus.levelUpSound = getPackId() + "_" + mId;
        })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new increment sound. This "
            "only "
            "applies to the particular level where this function is called.");

    addLuaFn(lua, "a_overrideSwapSound", //
        [this](const std::string& mId) {
            levelStatus.swapSound = getPackId() + "_" + mId;
        })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new swap sound. This only "
            "applies to the particular level where this function is called.");

    addLuaFn(lua, "a_overrideDeathSound", //
        [this](const std::string& mId) {
            levelStatus.deathSound = getPackId() + "_" + mId;
        })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "sets the specified file `$0` to be the new death sound. This only "
            "applies to the particular level where this function is called.");
}

void HexagonGame::initLua_MainTimeline()
{
    addLuaFn(lua, "t_eval",
        [this](const std::string& mCode) {
            timeline.append_do([=, this] { Utils::runLuaCode(lua, mCode); });
        })
        .arg("code")
        .doc(
            "*Add to the main timeline*: evaluate the Lua code specified in "
            "`$0`.");

    addLuaFn(lua, "t_clear", [this]() {
        timeline.clear();
    }).doc("Clear the main timeline.");

    addLuaFn(lua, "t_kill", //
        [this] { timeline.append_do([this] { death(true); }); })
        .doc("*Add to the main timeline*: kill the player.");

    addLuaFn(lua, "t_wait",
        [this](
            double mDuration) { timeline.append_wait_for_sixths(mDuration); })
        .arg("duration")
        .doc(
            "*Add to the main timeline*: wait for `$0` frames (under the "
            "assumption of a 60 FPS frame rate).");

    addLuaFn(lua, "t_waitS", //
        [this](
            double mDuration) { timeline.append_wait_for_seconds(mDuration); })
        .arg("duration")
        .doc("*Add to the main timeline*: wait for `$0` seconds.");

    addLuaFn(lua, "t_waitUntilS", //
        [this](double mDuration) {
            timeline.append_wait_until_fn([this, mDuration] {
                return status.getLevelStartTP() +
                       std::chrono::milliseconds(
                           static_cast<int>(mDuration * 1000.0));
            });
        })
        .arg("duration")
        .doc(
            "*Add to the main timeline*: wait until the timer reaches `$0` "
            "seconds.");
}

void HexagonGame::initLua_EventTimeline()
{
    addLuaFn(lua, "e_eval",
        [this](const std::string& mCode) {
            eventTimeline.append_do(
                [=, this] { Utils::runLuaCode(lua, mCode); });
        })
        .arg("code")
        .doc(
            "*Add to the event timeline*: evaluate the Lua code specified in "
            "`$0`. (This is the closest you'll get to 1.92 events)");

    addLuaFn(lua, "e_kill", //
        [this] { eventTimeline.append_do([this] { death(true); }); })
        .doc("*Add to the event timeline*: kill the player.");

    addLuaFn(lua, "e_stopTime", //
        [this](double mDuration) {
            eventTimeline.append_do([=, this] {
                status.pauseTime(ssvu::getFTToSeconds(mDuration));
            });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` frames "
            "(under the assumption of a 60 FPS frame rate).");

    addLuaFn(lua, "e_stopTimeS", //
        [this](double mDuration) {
            eventTimeline.append_do([=, this] { status.pauseTime(mDuration); });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` "
            "seconds.");

    addLuaFn(lua, "e_wait",
        [this](double mDuration) {
            eventTimeline.append_wait_for_sixths(mDuration);
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait for `$0` frames (under the "
            "assumption of a 60 FPS frame rate).");

    addLuaFn(lua, "e_waitS", //
        [this](double mDuration) {
            eventTimeline.append_wait_for_seconds(mDuration);
        })
        .arg("duration")
        .doc("*Add to the event timeline*: wait for `$0` seconds.");

    addLuaFn(lua, "e_waitUntilS", //
        [this](double mDuration) {
            eventTimeline.append_wait_until_fn([this, mDuration] {
                return status.getLevelStartTP() +
                       std::chrono::milliseconds(
                           static_cast<int>(mDuration * 1000.0));
            });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait until the timer reaches `$0` "
            "seconds.");

    addLuaFn(lua, "e_messageAdd", //
        [this](const std::string& mMsg, double mDuration) {
            eventTimeline.append_do([=, this] {
                if(firstPlay && Config::getShowMessages())
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

    addLuaFn(lua, "e_messageAddImportant", //
        [this](const std::string& mMsg, double mDuration) {
            eventTimeline.append_do([=, this] {
                if(Config::getShowMessages())
                {
                    addMessage(mMsg, mDuration, /* mSoundToggle */ true);
                }
            });
        })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will be printed during every run of the "
            "level.");

    addLuaFn(lua, "e_messageAddImportantSilent",
        [this](const std::string& mMsg, double mDuration) {
            eventTimeline.append_do([=, this] {
                if(Config::getShowMessages())
                {
                    addMessage(mMsg, mDuration, /* mSoundToggle */ false);
                }
            });
        })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will only be printed during every "
            "run of the level, and will not produce any sound.");

    addLuaFn(lua, "e_clearMessages", //
        [this] { clearMessages(); })
        .doc("Remove all previously scheduled messages.");
}

template <typename T>
auto HexagonGame::makeLuaAccessor(T& obj, const std::string& prefix)
{
    return [this, &obj, prefix](const std::string& name, auto pmd,
               const std::string& getterDesc, const std::string& setterDesc) {
        using Type = std::decay_t<decltype(obj.*pmd)>;

        const std::string getterString = prefix + "_get" + name;
        const std::string setterString = prefix + "_set" + name;

        addLuaFn(lua, getterString, //
            [this, pmd, &obj]() -> Type { return obj.*pmd; })
            .doc(getterDesc);

        addLuaFn(lua, setterString, //
            [this, pmd, &obj](Type mValue) { obj.*pmd = mValue; })
            .arg("value")
            .doc(setterDesc);
    };
}

void HexagonGame::initLua_LevelControl()
{
    const auto lsVar = makeLuaAccessor(levelStatus, "l");

    lsVar("SpeedMult", &LevelStatus::speedMult,
        "Gets the speed multiplier of the level. The speed multiplier is "
        "the current speed of the walls. Is incremented by ``SpeedInc`` "
        "every increment and caps at ``speedMax``.",

        "Sets the speed multiplier of the level to `$0`. Changes do not apply "
        "to "
        "all walls immediately, and changes apply as soon as the next wall "
        "is created.");

    lsVar("PlayerSpeedMult", &LevelStatus::playerSpeedMult,
        "Gets the speed multiplier of the player.",

        "Sets the speed multiplier of the player.");

    lsVar("SpeedInc", &LevelStatus::speedInc,
        "Gets the speed increment of the level. This is applied every level "
        "increment to the speed multiplier. Increments are additive.",

        "Sets the speed increment of the level to `$0`.");

    lsVar("SpeedMax", &LevelStatus::speedMax,
        "Gets the maximum speed of the level. This is the highest that speed "
        "can go; speed can not get any higher than this.",

        "Sets the maximum speed of the level to `$0`. Keep in mind that speed "
        "keeps going past the speed max, so setting a higher speed max may "
        "make the speed instantly increase to the max.");

    lsVar("RotationSpeed", &LevelStatus::rotationSpeed,
        "Gets the rotation speed of the level. Is incremented by "
        "``RotationSpeedInc`` every increment and caps at "
        "``RotationSpeedMax``.",

        "Sets the rotation speed of the level to `$0`. Changes apply "
        "immediately.");

    lsVar("RotationSpeedInc", &LevelStatus::rotationSpeedInc,
        "Gets the rotation speed increment of the level. This is "
        "applied every level increment to the rotation speed. "
        "Increments are additive.",

        "Sets the rotation speed increment of the level to `$0`. "
        "Is effective on the next level increment.");

    lsVar("RotationSpeedMax", &LevelStatus::rotationSpeedMax,
        "Gets the maximum rotation speed of the level. This is the "
        "highest that rotation speed can go; rotation speed can not "
        "get any higher than this.",

        "Sets the maximum rotation speed of the level to `$0`. Keep "
        "in mind that rotation speed keeps going past the max, so "
        "setting a higher rotation speed max may make the rotation speed "
        "instantly increase to the max.");

    lsVar("DelayMult", &LevelStatus::delayMult,
        "Gets the delay multiplier of the level. The delay multiplier "
        "is the multiplier used to assist in spacing patterns, especially "
        "in cases of higher / lower speeds.  Is incremented by ``DelayInc`` "
        "every increment and is clamped between ``DelayMin`` and ``DelayMax``",

        "Sets the delay multiplier of the level to `$0`. Changes do not apply "
        "to "
        "patterns immediately, and changes apply as soon as the next pattern "
        "is spawned.");

    lsVar("DelayInc", &LevelStatus::delayInc,
        "Gets the delay increment of the level. This is applied every level "
        "increment to the delay multiplier. Increments are additive.",

        "Sets the delay increment of the level to `$0`.");

    lsVar("DelayMin", &LevelStatus::delayMin,
        "Gets the minimum delay of the level. This is the lowest that delay "
        "can go; delay can not get any lower than this.",

        "Sets the minimum delay of the level to `$0`. Keep in mind that delay "
        "can go below the delay min, so setting a lower delay min may "
        "make the delay instantly decrease to the minimum.");

    lsVar("DelayMax", &LevelStatus::delayMax,
        "Gets the maximum delay of the level. This is the highest that delay "
        "can go; delay can not get any higher than this.",

        "Sets the maximum delay of the level to `$0`. Keep in mind that delay "
        "can go above the delay max, so setting a higher delay max may "
        "make the delay instantly increase to the maximum.");

    lsVar("FastSpin", &LevelStatus::fastSpin,
        "Gets the fast spin of the level. The fast spin is a brief moment that "
        "starts at level incrementation where the rotation increases speed "
        "drastically to try and throw off the player a bit. This speed quickly "
        "(or slowly, depending on the value) decelerates and fades away to the "
        " updated rotation speed.",

        "Sets the fast spin of the level to `$0`. A higher value increases "
        "intensity and duration of the fast spin.");

    lsVar("IncTime", &LevelStatus::incTime,
        "Get the incrementation time (in seconds) of a level. This is the "
        "length "
        "of a \"level\" in an Open Hexagon level (It's ambiguous but hopefully "
        "you understand what that means), and when this duration is reached, "
        "the "
        "level increments.",

        "Set the incrementation time (in seconds) of a level to `$0`.");

    lsVar("PulseMin", &LevelStatus::pulseMin,
        "Gets the minimum value the pulse can be. Pulse gives variety in "
        "the wall speed of the level so the wall speed doesn't feel monotone. "
        "Can also be used to help sync a level up with it's music.",

        "Sets the minimum pulse value to `$0`.");

    lsVar("PulseMax", &LevelStatus::pulseMax,
        "Gets the maximum value the pulse can be. Pulse gives variety in "
        "the wall speed of the level so the wall speed doesn't feel monotone. "
        "Can also be used to help sync a level up with it's music.",

        "Sets the maximum pulse value to `$0`.");

    lsVar("PulseSpeed", &LevelStatus::pulseSpeed,
        "Gets the speed the pulse goes from ``PulseMin`` to ``PulseMax``. "
        "Can also be used to help sync a level up with it's music.",

        "Sets the speed the pulse goes from ``PulseMin`` to ``PulseMax`` by "
        "`$0`. Can also be used to help sync a level up with it's music.");

    lsVar("PulseSpeedR", &LevelStatus::pulseSpeedR,
        "Gets the speed the pulse goes from ``PulseMax`` to ``PulseMin``.",

        "Sets the speed the pulse goes from ``PulseMax`` to ``PulseMin`` by "
        "`$0`. Can also be used to help sync a level up with it's music.");

    lsVar("PulseDelayMax", &LevelStatus::pulseDelayMax,
        "Gets the delay the level has to wait before it begins another pulse "
        "cycle.",

        "Sets the delay the level has to wait before it begins another pulse "
        "cycle with `$0`.");

    lsVar("PulseInitialDelay", &LevelStatus::pulseInitialDelay,
        "Gets the initial delay the level has to wait before it begins the "
        "first pulse cycle.",

        "Sets the initial delay the level has to wait before it begins the "
        "first pulse cycle with `$0`.");

    lsVar("SwapCooldownMult", &LevelStatus::swapCooldownMult,
        "Gets the multiplier that controls the cooldown for the player's 180 "
        "degrees swap mechanic.",

        "Sets the multiplier that controls the cooldown for the player's 180 "
        "degrees swap mechanic to `$0`.");

    lsVar("BeatPulseMax", &LevelStatus::beatPulseMax,
        "Gets the maximum beatpulse size of the polygon in a level. This is "
        "the highest value that the polygon will \"pulse\" in size. Useful for "
        "syncing the level to the music.",

        "Sets the maximum beatpulse size of the polygon in a level to `$0`. "
        "Not to be confused with using this property to resize the polygon, "
        "which you should be using ``RadiusMin``.");

    lsVar("BeatPulseDelayMax", &LevelStatus::beatPulseDelayMax,
        "Gets the delay for how fast the beatpulse pulses in frames (assuming "
        "60 FPS "
        "logic). This paired with ``BeatPulseMax`` will be useful to help sync "
        "a level "
        "with the music that it's playing.",

        "Sets the delay for how fast the beatpulse pulses in `$0` frames "
        "(assuming 60 "
        "FPS Logic).");

    lsVar("BeatPulseInitialDelay", &LevelStatus::beatPulseInitialDelay,
        "Gets the initial delay before beatpulse begins pulsing. This is very "
        "useful "
        "to use at the very beginning of the level to assist syncing the "
        "beatpulse "
        "with the song.",

        "Sets the initial delay before beatpulse begins pulsing to `$0`. "
        "Highly "
        "discouraged to use this here. Use this in your music JSON files.");

    lsVar("BeatPulseSpeedMult", &LevelStatus::beatPulseSpeedMult,
        "Gets how fast the polygon pulses with the beatpulse. This is very "
        "useful "
        "to help keep your level in sync with the music.",

        "Sets how fast the polygon pulses with beatpulse to `$0`.");

    lsVar("RadiusMin", &LevelStatus::radiusMin,
        "Gets the minimum radius of the polygon in a level. This is used to "
        "determine the absolute size of the polygon in the level.",

        "Sets the minimum radius of the polygon to `$0`. Use this to set the "
        "size of the polygon in the level, not ``BeatPulseMax``.");

    lsVar("WallSkewLeft", &LevelStatus::wallSkewLeft,
        "Gets the Y axis offset of the top left vertex in all walls.",

        "Sets the Y axis offset of the top left vertex to `$0` in all newly "
        "generated "
        "walls. If you would like to have more individual control of the wall "
        "vertices, "
        "please use the custom walls system under the prefix ``cw_``.");

    lsVar("WallSkewRight", &LevelStatus::wallSkewRight,
        "Gets the Y axis offset of the top right vertex in all walls.",

        "Sets the Y axis offset of the top right vertex to `$0` in all newly "
        "generated "
        "walls. If you would like to have more individual control of the wall "
        "vertices, "
        "please use the custom walls system under the prefix ``cw_``.");

    lsVar("WallAngleLeft", &LevelStatus::wallAngleLeft,
        "Gets the X axis offset of the top left vertex in all walls.",

        "Sets the X axis offset of the top left vertex to `$0` in all newly "
        "generated "
        "walls. If you would like to have more individual control of the wall "
        "vertices, "
        "please use the custom walls system under the prefix ``cw_``.");

    lsVar("WallAngleRight", &LevelStatus::wallAngleRight,
        "Gets the X axis offset of the top right vertex in all walls.",

        "Sets the X axis offset of the top right vertex to `$0` in all newly "
        "generated "
        "walls. If you would like to have more individual control of the wall "
        "vertices, "
        "please use the custom walls system under the prefix ``cw_``.");

    lsVar("WallSpawnDistance", &LevelStatus::wallSpawnDistance,
        "Gets the distance at which standard walls spawn.",

        "Sets how far away the walls can spawn from the center. Higher "
        "values make walls spawn farther away, and will increase the "
        "player's wait for incoming walls.");

    lsVar("3dRequired", &LevelStatus::_3DRequired,
        "Gets whether 3D must be enabled in order to have a valid score in "
        "this level. "
        "By default, this value is ``false``.",

        "Sets whether 3D must be enabled to `$0` to have a valid score. Only "
        "set this "
        "to ``true`` if your level relies on 3D effects to work as intended.");

    // Commenting this one out. This property seems to have NO USE in the actual
    // game itself. lsVar("3dEffectMultiplier",
    // &LevelStatus::_3dEffectMultiplier);

    lsVar("CameraShake", &LevelStatus::cameraShake,
        "Gets the intensity of the camera shaking in a level.",

        "Sets the intensity of the camera shaking in a level to `$0`. This "
        "remains "
        "permanent until you either set this to 0 or the player dies.");

    lsVar("Sides", &LevelStatus::sides,
        "Gets the current number of sides on the polygon in a level.",

        "Sets the current number of sides on the polygon to `$0`. This change "
        "happens "
        "immediately and previously spawned walls will not adjust to the new "
        "side count.");

    lsVar("SidesMax", &LevelStatus::sidesMax,
        "Gets the maximum range that the number of sides can possibly be at "
        "random. "
        "``enableRndSideChanges`` must be enabled for this property to have "
        "any use.",

        "Sets the maximum range that the number of sides can possibly be to "
        "`$0`.");

    lsVar("SidesMin", &LevelStatus::sidesMin,
        "Gets the minimum range that the number of sides can possibly be at "
        "random. "
        "``enableRndSideChanges`` must be enabled for this property to have "
        "any use.",

        "Sets the minimum range that the number of sides can possibly be to "
        "`$0`.");

    lsVar("SwapEnabled", &LevelStatus::swapEnabled,
        "Gets whether the swap mechanic is enabled for a level. By default, "
        "this is "
        "set to ``false``.",

        "Sets the swap mechanic's availability to `$0`.");

    lsVar("TutorialMode", &LevelStatus::tutorialMode,
        "Gets whether tutorial mode is enabled. In tutorial mode, players are "
        "granted "
        "invincibility from dying to walls. This mode is typically enabled "
        "whenever a "
        "pack developer needs to demonstrate a new concept to the player so "
        "that way "
        "they can easily learn the new mechanic/concept. This invincibility "
        "will not "
        "count towards invalidating a score, but it's usually not important to "
        "score "
        "on a tutorial level. By default, this is set to ``false``.",

        "Sets tutorial mode to `$0`. Remember, only enable this if you need to "
        "demonstrate "
        "a new concept for players to learn, or use it as a gimmick to a "
        "level.");

    lsVar("IncEnabled", &LevelStatus::incEnabled,
        "Gets whether the level can increment or not. This is Open Hexagon's "
        "way of "
        "establishing a difficulty curve in the level and set a sense of "
        "progression "
        "throughout the level. By default, this value is set to ``true``.",

        "Toggles level incrementation to `$0`. Only disable this if you feel "
        "like the "
        "level can not benefit from incrementing in any way.");

    lsVar("DarkenUnevenBackgroundChunk",
        &LevelStatus::darkenUnevenBackgroundChunk,
        "Gets whether the ``Nth`` panel of a polygon with ``N`` sides "
        "(assuming ``N`` is odd) will be darkened to make styles look more "
        "balanced. By default, this value is set to ``true``, but there can be "
        "styles where having this darkened panel can look very unpleasing.",

        "Sets the darkened panel to `$0`.");

    lsVar("ManualPulseControl", &LevelStatus::manualPulseControl,
        "Gets whether the pulse effect is being controlled manually via Lua or "
        "automatically by the C++ engine.",

        "Sets whether the pulse effect is being controlled manually via Lua or "
        "automatically by the C++ engine to `$0`.");

    lsVar("ManualBeatPulseControl", &LevelStatus::manualBeatPulseControl,
        "Gets whether the beat pulse effect is being controlled manually via "
        "Lua or automatically by the C++ engine.",

        "Sets whether the beat  pulse effect is being controlled manually via "
        "Lua or automatically by the C++ engine to `$0`.");

    lsVar("CurrentIncrements", &LevelStatus::currentIncrements,
        "Gets the current amount of times the level has incremented. Very "
        "useful for "
        "keeping track of levels.",

        "Sets the current amount of times the level has incremented to `$0`. "
        "This "
        "function is utterly pointless to use unless you are tracking this "
        "variable.");

    addLuaFn(lua, "l_enableRndSideChanges", //
        [this](bool mValue) { levelStatus.rndSideChangesEnabled = mValue; })
        .arg("enabled")
        .doc(
            "Toggles random side changes to `$0`, (not) allowing sides to "
            "change "
            "between ``SidesMin`` and ``SidesMax`` inclusively every level "
            "increment.");

    addLuaFn(lua, "l_overrideScore", //
        [this](const std::string& mVar) {
            try
            {
                levelStatus.scoreOverride = mVar;
                levelStatus.scoreOverridden = true;
                // Make sure we're not passing in a string
                lua.executeCode("if (type(" + mVar + R"( ) ~= "number") then
								error("Score override must be a number value")
								end
)");
            }
            catch(const std::runtime_error& mError)
            {
                std::cout
                    << "[l_overrideScore] Runtime error on overriding score "
                    << "with level \"" << levelData->name << "\": \n"
                    << ssvu::toStr(mError.what()) << '\n'
                    << std::endl;
                if(!Config::getDebug())
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

    addLuaFn(lua, "l_addTracked", //
        [this](const std::string& mVar, const std::string& mName) {
            levelStatus.trackedVariables.push_back({mVar, mName});
        })
        .arg("variable")
        .arg("name")
        .doc(
            "Add the variable `$0` to the list of tracked variables, with name "
            "`$1`. Tracked variables are displayed in game, below the game "
            "timer. *NOTE: Your variable must be global for this to work.*");

    addLuaFn(lua, "l_clearTracked", //
        [this] { levelStatus.trackedVariables.clear(); })
        .doc("Clears all tracked variables.");

    addLuaFn(lua, "l_setRotation", //
        [this](float mValue) {
            // TODO: might break replays
            if(backgroundCamera.has_value())
            {
                backgroundCamera->setRotation(mValue);
            }
        })
        .arg("angle")
        .doc("Set the background camera rotation to `$0` degrees.");

    addLuaFn(lua, "l_getRotation", //
        [this] {
            // TODO: might break replays
            return backgroundCamera.has_value()
                       ? backgroundCamera->getRotation()
                       : 0.f;
        })
        .doc("Return the background camera rotation, in degrees.");

    addLuaFn(lua, "l_getLevelTime", //
        [this] { return status.getTimeSeconds(); })
        .doc("Get the current game timer value, in seconds.");

    addLuaFn(lua, "l_getOfficial", //
        [] { return Config::getOfficial(); })
        .doc(
            "Return `true` if \"official mode\" is enabled, `false` "
            "otherwise.");

    addLuaFn(lua, "l_resetTime", //
        [this] { status.resetTime(); })
        .doc(
            "Resets the lever time to zero, also resets increment time and "
            "pause time.");

    const auto sVar = makeLuaAccessor(status, "l");

    sVar("Pulse", &HexagonGameStatus::pulse,
        "Gets the current pulse value, which will vary between "
        "`l_getPulseMin()` and `l_getPulseMax()` unless manually overridden.",

        "Sets the current pulse value to `$0`.");

    sVar("PulseDirection", &HexagonGameStatus::pulseDirection,
        "Gets the current pulse direction value, which will either be `-1` or "
        "`1` unless manually overridden.",

        "Sets the current pulse direction value to `$0`. Valid choices are "
        "`-1` or `1`.");

    sVar("PulseDelay", &HexagonGameStatus::pulseDelay,
        "Gets the current pulse delay value, which will vary between "
        "`0` and `l_getPulseDelayMax()` unless manually overridden.",

        "Sets the current pulse delay value to `$0`.");

    sVar("BeatPulse", &HexagonGameStatus::beatPulse,
        "Gets the current beat pulse value, which will vary between `0` and "
        "`l_getBeatPulseMax()` unless manually overridden.",

        "Sets the current beat pulse value to `$0`.");

    sVar("BeatPulseDelay", &HexagonGameStatus::beatPulseDelay,
        "Gets the current beat pulse delay value, which will vary between "
        "`0` and `l_getBeatPulseDelayMax()` unless manually overridden.",

        "Sets the current beat pulse delay value to `$0`.");
}

void HexagonGame::initLua_StyleControl()
{
    const auto sdVar = makeLuaAccessor(styleData, "s");

    sdVar("HueMin", &StyleData::hueMin,
        "Gets the minimum value for the hue range of a level style. The hue "
        "attribute is an important attribute that is dedicated specifically to "
        "all colors that have the ``dynamic`` property enabled.",

        "Sets the minimum value for the hue range to `$0`. Usually you want "
        "this value at 0 to start off at completely red.");

    sdVar("HueMax", &StyleData::hueMax,
        "Gets the maximum value for the hue range of a level style. Only "
        "applies to all colors with the ``dynamic`` property enabled.",

        "Sets the maximum value for the hue range to `$0`. Usually you want "
        "this value at 360 to end off at red, to hopefully loop the colors "
        "around.");

    // backwards-compatible
    sdVar("HueInc", &StyleData::hueIncrement,
        "Alias to ``s_getHueIncrement``. Done for backwards compatibility.",

        "Alias to ``s_setHueIncrement``. Done for backwards compatibility.");

    sdVar("HueIncrement", &StyleData::hueIncrement,
        "Gets how fast the hue increments from ``HueMin`` to ``HueMax``. The "
        "hue value is "
        "added by this value every 1/60th of a second.",

        "Sets how fast the hue increments from ``HueMin`` to ``HueMax`` by "
        "`$0`. Be careful "
        "with high values, as this can make your style induce epileptic "
        "seizures.");

    sdVar("PulseMin", &StyleData::pulseMin,
        "Gets the minimum range for the multiplier of the ``pulse`` attribute "
        "in style colors. "
        "By default, this value is set to 0.",

        "Sets the minimum range for the multiplier of the ``pulse`` attribute "
        "to `$0`.");

    sdVar("PulseMax", &StyleData::pulseMax,
        "Gets the maximum range for the multiplier of the ``pulse`` attribute "
        "in style colors. "
        "By default, this value is set to 0, but ideally it should be set to "
        "1.",

        "Sets the maximum range for the multiplier of the ``pulse`` attribute "
        "to `$0`.");

    // backwards-compatible
    sdVar("PulseInc", &StyleData::pulseIncrement,
        "Alias to ``s_getPulseIncrement``. Done for backwards compatibility.",

        "Alias to ``s_setPulseIncrement``. Done for backwards compatibility.");

    sdVar("PulseIncrement", &StyleData::pulseIncrement,
        "Gets how fast the pulse increments from ``PulseMin`` to ``PulseMax``. "
        "The pulse value is "
        "added by this value every 1/60th of a second.",

        "Sets how fast the pulse increments from ``PulseMin`` to ``PulseMax`` "
        "by `$0`. Be careful "
        "with high values, as this can make your style induce epileptic "
        "seizures.");

    sdVar("HuePingPong", &StyleData::huePingPong,
        "Gets whether the hue should go ``Start-End-Start-End`` or "
        "``Start-End, Start-End`` with "
        "the hue cycling.",

        "Toggles ping ponging in the hue cycling (``Start-End-Start-End``) "
        "with `$0`.");

    sdVar("MaxSwapTime", &StyleData::maxSwapTime,
        "Gets the amount of time that has to pass (in 1/100th of a second) "
        "before the background color offset alternates. "
        "The background colors by default alternate between 0 and 1. By "
        "default, this happens every second.",

        "Sets the amount of time that has to pass (in 1/100th of a second) to "
        "`$0` before the background color alternates.");

    sdVar("3dDepth", &StyleData::_3dDepth,
        "Gets the current amount of 3D layers that are present in the style.",

        "Sets the amount of 3D layers in a style to `$0`.");

    sdVar("3dSkew", &StyleData::_3dSkew,
        "Gets the current value of where the 3D skew is in the style. The Skew "
        "is what gives the 3D effect in the first "
        "place, showing the 3D layers and giving the illusion of 3D in the "
        "game.",

        "Sets the 3D skew at value `$0`.");

    sdVar("3dSpacing", &StyleData::_3dSpacing,
        "Gets the spacing that is done between 3D layers. A higher number "
        "leads to more separation between layers.",

        "Sets the spacing between 3D layers to `$0`.");

    sdVar("3dDarkenMult", &StyleData::_3dDarkenMult,
        "Gets the darkening multiplier applied to the 3D layers in a style. "
        "This is taken from the ``main`` color.",

        "Sets the darkening multiplier to `$0` for the 3D layers.");

    sdVar("3dAlphaMult", &StyleData::_3dAlphaMult,
        "Gets the alpha (transparency) multiplier applied to the 3D layers in "
        "a style. Originally references the "
        "``main`` color.",

        "Sets the alpha multiplier to `$0` for the 3D layers. A higher value "
        "makes the layers more transparent.");

    sdVar("3dAlphaFalloff", &StyleData::_3dAlphaFalloff,
        "Gets the alpha (transparency) multiplier applied to the 3D layers "
        "consecutively in a style. Takes "
        "reference from the ``main`` color.",

        "Sets the alpha multiplier to `$0` for for the 3D layers and applies "
        "them layer after layer. This "
        "property can get finnicky.");

    sdVar("3dPulseMax", &StyleData::_3dPulseMax,
        "Gets the highest value that the ``3DSkew`` can go in a style.",

        "Sets the highest value the ``3DSkew`` can go to `$0`.");

    sdVar("3dPulseMin", &StyleData::_3dPulseMin,
        "Gets the lowest value that the ``3DSkew`` can go in a style.",

        "Sets the lowest value the ``3DSkew`` can go to `$0`.");

    sdVar("3dPulseSpeed", &StyleData::_3dPulseSpeed,
        "Gets how fast the ``3DSkew`` moves between ``3DPulseMin`` and "
        "``3DPulseMax``.",

        "Sets how fast the ``3DSkew`` moves between ``3DPulseMin`` and "
        "``3DPulseMax`` by `$0`.");

    sdVar("3dPerspectiveMult", &StyleData::_3dPerspectiveMult,
        "Gets the 3D perspective multiplier of the style. Works with the "
        "attribute ``3DSpacing`` to space out "
        "layers.",

        "Sets the 3D perspective multiplier to `$0`.");

    sdVar("BGTileRadius", &StyleData::bgTileRadius,
        "Gets the distances of how far the background panels are drawn. By "
        "default, this is a big enough value "
        "so you do not see the border. However, feel free to shrink them if "
        "you'd like.",

        "Sets how far the background panels are drawn to distance `$0`.");

    sdVar("BGColorOffset", &StyleData::BGColorOffset,
        "Gets the offset of the style by how much the colors shift. Usually "
        "this sits between 0 and 1, but can "
        "easily be customized.",

        "Shifts the background colors to have an offset of `$0`.");

    sdVar("BGRotationOffset", &StyleData::BGRotOff,
        "Gets the literal rotation offset of the background panels in degrees. "
        "This usually stays at 0, but can "
        "be messed with to make some stylish level styles.",

        "Sets the rotation offset of the background panels to `$0` degrees.");

    addLuaFn(lua, "s_setStyle", //
        [this](const std::string& mId) {
            styleData = assets.getStyleData(levelData->packId, mId);
            styleData.computeColors();
        })
        .arg("styleId")
        .doc(
            "Set the currently active style to the style with id `$0`. Styles "
            "can be defined as `.json` files in the `<pack>/Styles/` folder.");

    // // backwards-compatible
    // addLuaFn(lua, "s_setCameraShake", //
    //     [this](int mValue) { levelStatus.cameraShake = mValue; })
    //     .arg("value")
    //     .doc("Start a camera shake with intensity `$0`.");

    // // backwards-compatible
    // addLuaFn(lua, "s_getCameraShake", //
    //     [this] { return levelStatus.cameraShake; })
    //     .doc("Return the current camera shake intensity.");

    addLuaFn(lua, "s_setCapColorMain", //
        [this] { styleData.capColor = CapColorMode::Main{}; })
        .doc(
            "Set the color of the center polygon to match the main style "
            "color.");

    addLuaFn(lua, "s_setCapColorMainDarkened", //
        [this] { styleData.capColor = CapColorMode::MainDarkened{}; })
        .doc(
            "Set the color of the center polygon to match the main style "
            "color, darkened.");

    addLuaFn(lua, "s_setCapColorByIndex", //
        [this](
            int mIndex) { styleData.capColor = CapColorMode::ByIndex{mIndex}; })
        .arg("index")
        .doc(
            "Set the color of the center polygon to match the style color with "
            "index `$0`.");

    const auto colorToTuple = [](const sf::Color& c) {
        return std::tuple<int, int, int, int>{c.r, c.g, c.b, c.a};
    };

    // TODO:
    const auto sdColorGetter = [this, &colorToTuple](const char* name,
                                   const char* docName, auto pmf) {
        std::string docString = "Return the current ";
        docString += docName;
        docString += " color computed by the level style.";

        addLuaFn(lua, name, [this, &colorToTuple, pmf] {
            return colorToTuple((styleData.*pmf)());
        }).doc(docString);
    };

    sdColorGetter("s_getMainColor", "main", &StyleData::getMainColor);
    sdColorGetter("s_getPlayerColor", "player", &StyleData::getPlayerColor);
    sdColorGetter("s_getTextColor", "text", &StyleData::getTextColor);

    sdColorGetter(
        "s_get3DOverrideColor", "3D override", &StyleData::get3DOverrideColor);

    sdColorGetter("s_getCapColorResult", "cap color result",
        &StyleData::getCapColorResult);

    addLuaFn(lua, "s_getColor",
        [this, &colorToTuple](int mIndex) {
            if(styleData.getColors().empty())
            {
                return colorToTuple(sf::Color{0, 0, 0, 0});
            }

            return colorToTuple(styleData.getColor(mIndex));
        })
        .arg("index")
        .doc(
            "Return the current color with index `$0` computed by the level "
            "style.");
}

void HexagonGame::initLua_WallCreation()
{
    addLuaFn(lua, "w_wall", //
        [this](int mSide, float mThickness) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness, SpeedData{getSpeedMultDM()});
            });
        })
        .arg("side")
        .arg("thickness")
        .doc(
            "Create a new wall at side `$0`, with thickness `$1`. The speed of "
            "the wall will be calculated by using the speed multiplier, "
            "adjusted for the current difficulty multiplier.");

    addLuaFn(lua, "w_wallAdj", //
        [this](int mSide, float mThickness, float mSpeedAdj) {
            timeline.append_do([=, this] {
                createWall(
                    mSide, mThickness, SpeedData{mSpeedAdj * getSpeedMultDM()});
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

    addLuaFn(lua, "w_wallAcc", //
        [this](int mSide, float mThickness, float mSpeedAdj,
            float mAcceleration, float mMinSpeed, float mMaxSpeed) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness,
                    SpeedData{mSpeedAdj * getSpeedMultDM(),
                        mAcceleration / (std::pow(difficultyMult, 0.65f)),
                        mMinSpeed * getSpeedMultDM(),
                        mMaxSpeed * getSpeedMultDM()});
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

    addLuaFn(lua, "w_wallHModSpeedData", //
        [this](float mHMod, int mSide, float mThickness, float mSAdj,
            float mSAcc, float mSMin, float mSMax, bool mSPingPong) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness,
                    SpeedData{mSAdj * getSpeedMultDM(), mSAcc, mSMin, mSMax,
                        mSPingPong},
                    SpeedData{}, mHMod);
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

    addLuaFn(lua, "w_wallHModCurveData", //
        [this](float mHMod, int mSide, float mThickness, float mCAdj,
            float mCAcc, float mCMin, float mCMax, bool mCPingPong) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness, SpeedData{getSpeedMultDM()},
                    SpeedData{mCAdj, mCAcc, mCMin, mCMax, mCPingPong}, mHMod);
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
    addLuaFn(lua, "steam_unlockAchievement", //
        [this](const std::string& mId) {
            if(inReplay())
            {
                // Do not unlock achievements while watching a replay.
                return;
            }

            if(steamManager != nullptr && Config::getOfficial())
            {
                steamManager->unlock_achievement(mId);
            }
        })
        .arg("achievementId")
        .doc("Unlock the Steam achievement with id `$0`.");
}

// These are all deprecated functions that are only being kept for the sake of
// lessening the impact of incompatibility. Pack Developers have time to change
// to the new functions before they get removed permanently
void HexagonGame::initLua_Deprecated()
{
    addLuaFn(lua, "u_kill", //
        [this] {
            raiseWarning("u_kill",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"t_kill\" in your level files.");
            timeline.append_do([this] { death(true); });
        })
        .doc(
            "*Add to the main timeline*: kill the player. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use t_kill instead!**");

    addLuaFn(lua, "u_eventKill", //
        [this] {
            raiseWarning("u_eventKill",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_kill\" in your level files.");
            eventTimeline.append_do([this] { death(true); });
        })
        .doc(
            "*Add to the event timeline*: kill the player. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_kill instead!**");

    addLuaFn(lua, "u_playSound", //
        [this](const std::string& mId) {
            raiseWarning("u_playSound",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"a_playSound\" in your level files.");
            playSoundOverride(mId);
        })
        .arg("soundId")
        .doc(
            "Play the sound with id `$0`. The id must be registered in "
            "`assets.json`, under `\"soundBuffers\"`. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use a_playSound instead!**");

    addLuaFn(lua, "u_playPackSound", //
        [this](const std::string& fileName) {
            raiseWarning("u_playPackSound",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"a_playPackSound\" in your level files.");
            playPackSoundOverride(getPackId(), fileName);
        })
        .arg("fileName")
        .doc(
            "Dives into the `Sounds` folder of the current level pack and "
            "plays the specified file `$0`. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use a_playPackSound instead!**");

    addLuaFn(lua, "e_eventStopTime", //
        [this](double mDuration) {
            raiseWarning("e_eventStopTime",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_stopTime\" in your level files.");
            eventTimeline.append_do([=, this] {
                status.pauseTime(ssvu::getFTToSeconds(mDuration));
            });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` frames "
            "(under the assumption of a 60 FPS frame rate). "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_stopTime instead!**");

    addLuaFn(lua, "e_eventStopTimeS", //
        [this](double mDuration) {
            raiseWarning("e_eventStopTimeS",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_stopTimeS\" in your level files.");
            eventTimeline.append_do([=, this] { status.pauseTime(mDuration); });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` "
            "seconds. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_stopTimeS instead!**");

    addLuaFn(lua, "e_eventWait",
        [this](double mDuration) {
            raiseWarning("e_eventWait",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_wait\" in your level files.");
            eventTimeline.append_wait_for_sixths(mDuration);
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait for `$0` frames (under the "
            "assumption of a 60 FPS frame rate). "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_wait instead!**");

    addLuaFn(lua, "e_eventWaitS", //
        [this](double mDuration) {
            raiseWarning("e_eventWaitS",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_waitS\" in your level files.");
            eventTimeline.append_wait_for_seconds(mDuration);
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait for `$0` seconds. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_waitS instead!**");

    addLuaFn(lua, "e_eventWaitUntilS", //
        [this](double mDuration) {
            raiseWarning("e_eventWaitUntilS",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_waitUntilS\" in your level files.");
            eventTimeline.append_wait_until_fn([this, mDuration] {
                return status.getLevelStartTP() +
                       std::chrono::milliseconds(
                           static_cast<int>(mDuration * 1000.0));
            });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: wait until the timer reaches `$0` "
            "seconds. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_waitUntilS instead!**");

    addLuaFn(lua, "m_messageAdd", //
        [this](const std::string& mMsg, double mDuration) {
            raiseWarning("m_messageAdd",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_messageAdd\" in your level files and common.lua.");
            eventTimeline.append_do([=, this] {
                if(firstPlay && Config::getShowMessages())
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
            "run of the level. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_messageAdd instead!**");

    addLuaFn(lua, "m_messageAddImportant", //
        [this](const std::string& mMsg, double mDuration) {
            raiseWarning("m_messageAddImportant",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_messageAddImportant\" in your level files and "
                "common.lua.");
            eventTimeline.append_do([=, this] {
                if(Config::getShowMessages())
                {
                    addMessage(mMsg, mDuration, /* mSoundToggle */ true);
                }
            });
        })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will be printed during every run of the "
            "level. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_messageAddImportant instead!**");

    addLuaFn(lua, "m_messageAddImportantSilent",
        [this](const std::string& mMsg, double mDuration) {
            raiseWarning("m_messageAddImportantSilent",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_messageAddImportantSilent\" in your level files.");
            eventTimeline.append_do([=, this] {
                if(Config::getShowMessages())
                {
                    addMessage(mMsg, mDuration, /* mSoundToggle */ false);
                }
            });
        })
        .arg("message")
        .arg("duration")
        .doc(
            "*Add to the event timeline*: print a message with text `$0` for "
            "`$1` seconds. The message will only be printed during every "
            "run of the level, and will not produce any sound. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_messageAddImportantSilent instead!**");

    addLuaFn(lua, "m_clearMessages", //
        [this] {
            raiseWarning("m_clearMessages",
                "This function will be removed in a future version of Open "
                "Hexagon. Please replace all occurrences of this function with "
                "\"e_clearMessages\" in your level files.");
            clearMessages();
        })
        .doc(
            "Remove all previously scheduled messages. "
            "**This function is deprecated and will be removed in a future "
            "version. Please use e_clearMessages instead!**");
}

void HexagonGame::initLua()
{
    LuaScripting::init(lua, rng, false /* inMenu */, cwManager);

    initLua_Utils();
    initLua_AudioControl();
    initLua_MainTimeline();
    initLua_EventTimeline();
    initLua_LevelControl();
    initLua_StyleControl();
    initLua_WallCreation();
    initLua_Steam();
    initLua_Deprecated();
}

void HexagonGame::initLuaAndPrintDocs()
{
    initLua();
    LuaScripting::printDocs();
}

} // namespace hg

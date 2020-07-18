// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Components/CCustomWall.hpp"

using namespace sf;
using namespace ssvs;
using namespace ssvuj;

namespace hg
{

void HexagonGame::initLua_Utils()
{
    addLuaFn("u_getAttemptRandomSeed", //
        [this] { return rng.get_seed(); })
        .doc(
            "Obtain the current random seed, automatically generated at the "
            "beginning of the level. `math.randomseed` is automatically "
            "initialized with the result of this function at the beginning of "
            "a level.");

    addLuaFn("u_log", //
        [this](std::string mLog) { ssvu::lo("lua") << mLog << "\n"; })
        .arg("message")
        .doc("Print out `$0` to the console.");

    addLuaFn("u_execScript", //
        [this](std::string mName) {
            runLuaFile(levelData->packPath + "Scripts/" + mName);
        })
        .arg("scriptFilename")
        .doc("Execute the script located at `<pack>/Scripts/$0`.");

    addLuaFn("u_playSound", //
        [this](std::string mId) { assets.playSound(mId); })
        .arg("soundId")
        .doc(
            "Play the sound with id `$0`. The id must be registered in "
            "`assets.json`, under `\"soundBuffers\"`.");

    addLuaFn("u_setMusic", //
        [this](std::string mId) {
            musicData = assets.getMusicData(levelData->packId, mId);
            musicData.firstPlay = true;
            stopLevelMusic();
            playLevelMusic();
        })
        .arg("musicId")
        .doc(
            "Stop the current music and play the music with id `$0`. The id is "
            "defined in the music `.json` file, under `\"id\"`.");

    addLuaFn("u_setMusicSegment", //
        [this](std::string mId, int segment) {
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

    addLuaFn("u_setMusicSeconds", //
        [this](std::string mId, float mTime) {
            musicData = assets.getMusicData(levelData->packId, mId);
            stopLevelMusic();
            playLevelMusicAtTime(mTime);
        })
        .arg("musicId")
        .arg("time")
        .doc(
            "Stop the current music and play the music with id `$0`, starting "
            "at time `$1` (in seconds).");

    addLuaFn("u_isKeyPressed",
        [this](int mKey) { return window.getInputState()[KKey(mKey)]; })
        .arg("keyCode")
        .doc(
            "Return `true` if the keyboard key with code `$0` is being "
            "pressed, `false` otherwise. The key code must match the "
            "definition of the SFML `sf::Keyboard::Key` enumeration.");

    addLuaFn("u_haltTime", //
        [this](double mDuration) {
            status.pauseTime(ssvu::getFTToSeconds(mDuration));
        })
        .arg("duration")
        .doc("Pause the game timer for `$0` seconds.");

    addLuaFn("u_timelineWait",
        [this](
            double mDuration) { timeline.append_wait_for_sixths(mDuration); })
        .arg("duration")
        .doc("*Add to the main timeline*: wait for `$0` sixths of a second.");

    addLuaFn("u_clearWalls", //
        [this] { walls.clear(); })
        .doc("Remove all existing walls.");

    addLuaFn("u_getPlayerAngle", //
        [this] { return player.getPlayerAngle(); })
        .doc("Return the current angle of the player, in radians.");

    addLuaFn("u_setPlayerAngle",
        [this](float newAng) { player.setPlayerAngle(newAng); })
        .arg("angle")
        .doc("Set the current angle of the player to `$0`, in radians.");

    addLuaFn("u_isMouseButtonPressed",
        [this](int mKey) { return window.getInputState()[MBtn(mKey)]; })
        .arg("buttonCode")
        .doc(
            "Return `true` if the mouse button with code `$0` is being "
            "pressed, `false` otherwise. The button code must match the "
            "definition of the SFML `sf::Mouse::Button` enumeration.");

    addLuaFn("u_isFastSpinning", //
        [this] { return status.fastSpin > 0; })
        .doc(
            "Return `true` if the camera is currently \"fast spinning\", "
            "`false` otherwise.");

    addLuaFn("u_forceIncrement", //
        [this] { incrementDifficulty(); })
        .doc(
            "Immediately force a difficulty increment, regardless of the "
            "chosen automatic increment parameters.");

    addLuaFn("u_kill", //
        [this] { timeline.append_do([this] { death(true); }); })
        .doc("*Add to the main timeline*: kill the player.");

    addLuaFn("u_eventKill", //
        [this] { eventTimeline.append_do([this] { death(true); }); })
        .doc("*Add to the event timeline*: kill the player.");

    addLuaFn("u_getDifficultyMult", //
        [this] { return difficultyMult; })
        .doc("Return the current difficulty multiplier.");

    addLuaFn("u_getSpeedMultDM", //
        [this] { return getSpeedMultDM(); })
        .doc(
            "Return the current speed multiplier, adjusted for the chosen "
            "difficulty multiplier.");

    addLuaFn("u_getDelayMultDM", //
        [this] { return getDelayMultDM(); })
        .doc(
            "Return the current delay multiplier, adjusted for the chosen "
            "difficulty multiplier.");

    addLuaFn("u_swapPlayer", //
        [this](bool mPlaySound) { player.playerSwap(*this, mPlaySound); })
        .arg("playSound")
        .doc(
            "Force-swaps (180 degrees) the player when invoked. If `$0` is "
            "`true`, the swap sound will be played.");
}

void HexagonGame::initLua_Messages()
{
    addLuaFn("m_messageAdd", //
        [this](std::string mMsg, double mDuration) {
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

    addLuaFn("m_messageAddImportant", //
        [this](std::string mMsg, double mDuration) {
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


    addLuaFn("m_messageAddImportantSilent",
        [this](std::string mMsg, double mDuration) {
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


    addLuaFn("m_clearMessages", //
        [this] { clearMessages(); })
        .doc("Remove all previously scheduled messages.");
}

void HexagonGame::initLua_MainTimeline()
{
    addLuaFn("t_wait",
        [this](
            double mDuration) { timeline.append_wait_for_sixths(mDuration); })
        .arg("duration")
        .doc("*Add to the main timeline*: wait for `$0` frames (under the assumption of a 60 FPS frame rate).");

    addLuaFn("t_waitS", //
        [this](
            double mDuration) { timeline.append_wait_for_seconds(mDuration); })
        .arg("duration")
        .doc("*Add to the main timeline*: wait for `$0` seconds.");

    addLuaFn("t_waitUntilS", //
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
    addLuaFn("e_eventStopTime", //
        [this](double mDuration) {
            eventTimeline.append_do([=, this] {
                status.pauseTime(ssvu::getFTToSeconds(mDuration));
            });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` frames (under the assumption of a 60 FPS frame rate).");

    addLuaFn("e_eventStopTimeS", //
        [this](double mDuration) {
            eventTimeline.append_do([=, this] { status.pauseTime(mDuration); });
        })
        .arg("duration")
        .doc(
            "*Add to the event timeline*: pause the game timer for `$0` "
            "seconds.");

    addLuaFn("e_eventWait",
        [this](double mDuration) {
            eventTimeline.append_wait_for_sixths(mDuration);
        })
        .arg("duration")
        .doc("*Add to the event timeline*: wait for `$0` sixths of a second.");

    addLuaFn("e_eventWaitS", //
        [this](double mDuration) {
            eventTimeline.append_wait_for_seconds(mDuration);
        })
        .arg("duration")
        .doc("*Add to the event timeline*: wait for `$0` seconds.");

    addLuaFn("e_eventWaitUntilS", //
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
}

void HexagonGame::initLua_LevelControl()
{
    const auto lsVar = [this](const std::string& name, auto pmd, 
                                const std::string& getterDesc, 
                                const std::string& setterDesc) {
        using Type = std::decay_t<decltype(levelStatus.*pmd)>;

        std::String getterString = std::string{"l_get"} + name;
        std::String setterString = std::string{"l_set"} + name;

        addLuaFn(getterString, //
            [this, pmd]() -> Type { return levelStatus.*pmd; })
            .doc(getterDesc);

        addLuaFn(setterString, //
            [this, pmd](Type mValue) { levelStatus.*pmd = mValue; })
            .arg("value")
            .doc(setterDesc);
    };

    lsVar("SpeedMult", &LevelStatus::speedMult,
        "Gets the speed multiplier of the level. The speed multiplier is "
        "the current speed of the walls. Is incremented by ``SpeedInc`` "
        "every increment and caps at ``speedMax``.",

        "Sets the speed multiplier of the level to `$0`. Changes do not apply to "
        "all walls immediately, and changes apply as soon as the next wall "
        "is created.");
    lsVar("SpeedInc", &LevelStatus::speedInc,
        "Gets the speed increment of the level. This is applied every level "
        "increment to the speed multiplier. Incrementation is additive.",
        
        "Sets the speed increment of the level to `$0`.");
    lsVar("SpeedMax", &LevelStatus::speedMax,
        "Gets the maximum speed of the level. This is the highest that speed "
        "can go; speed can not get any higher than this.",
        
        "Sets the maximum speed of the level to `$0`. Keep in mind that speed "
        "keeps going past the speed max, so setting a higher speed max may "
        "make the speed instantly increase to the max.");
    lsVar("RotationSpeed", &LevelStatus::rotationSpeed,
        "Gets the rotation speed of the level. Is incremented by "
        "``RotationSpeedInc`` every increment and caps at ``RotationSpeedMax``.",
        
        "Sets the rotation speed of the level to `$s0`. Changes apply "
        "immediately.");
    lsVar("RotationSpeedInc", &LevelStatus::rotationSpeedInc,
        "Gets the rotation speed increment of the level. This is "
        "applied every level increment to the rotation speed. "
        "Incrementation is additive.",
        
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
        
        "Sets the delay multiplier of the level to `$0`. Changes do not apply to "
        "patterns immediately, and changes apply as soon as the next pattern "
        "is spawned.");
    lsVar("DelayInc", &LevelStatus::delayInc,
        "Gets the delay increment of the level. This is applied every level "
        "increment to the delay multiplier. Incrementation is additive.",
        
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
        "Get the incrementation time (in seconds) of a level. This is the length "
        "of a \"level\" in an Open Hexagon level (It's ambiguous but hopefully "
        "you understand what that means), and when this duration is reached, the "
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

        "Gets the speed the pulse goes from ``PulseMin`` to ``PulseMax`` by "
        "`$0`. Can also be used to help sync a level up with it's music.");
    lsVar("PulseSpeedR", &LevelStatus::pulseSpeedR,
        "Gets the speed the pulse goes from ``PulseMax`` to ``PulseMin``.",

        "Gets the speed the pulse goes from ``PulseMax`` to ``PulseMin`` by "
        "`$0`. Can also be used to help sync a level up with it's music.");
    lsVar("PulseDelayMax", &LevelStatus::pulseDelayMax,
        "Gets the delay the level has to wait before it begins another pulse cycle.",
    
        "Sets the delay the level has to wait before it begins another pulse cycle "
        "with `$0`.");
    // TODO: Repurpose PulseDelayHalfMax to do what is listed on this documentation
    lsVar("PulseDelayHalfMax", &LevelStatus::pulseDelayHalfMax,
        "Gets the delay the level has to wait before it begins pulsing from "
        "``PulseMax`` to ``PulseMin``.",
    
        "Gets the delay the level has to wait before it begins pulsing from "
        "``PulseMax`` to ``PulseMin`` with `$0`.");
    lsVar("BeatPulseMax", &LevelStatus::beatPulseMax,
        "Gets the maximum beatpulse size of the polygon in a level. This is the "
        "highest value that the polygon will \"pulse\" in size. Useful for syncing "
        "the level to the music.",
    
        "Sets the maximum beatpulse size of the polygon in a level to `$s0`. Not "
        "to be confused with using this property to resize the polygon, which you "
        "should be using ``RadiusMin``.");
    lsVar("BeatPulseDelayMax", &LevelStatus::beatPulseDelayMax,
        "Gets the delay for how fast the beatpulse pulses in frames (assuming 60 FPS "
        "logic). This paired with ``BeatPulseMax`` will be useful to help sync a level "
        "with the music that it's playing.",
    
        "Sets the delay for how fast the beatpulse pulses in `$0` frames (assuming 60 "
        "FPS Logic).");
    lsVar("BeatPulseInitialDelay", &LevelStatus::beatPulseInitialDelay,
        "Gets the initial delay before beatpulse begins pulsing. This is very useful "
        "to use at the very beginning of the level to assist syncing the beatpulse "
        "with the song.",
        
        "Sets the initial delay before beatpulse begins pulsing to `$0`. Highly "
        "discouraged to use this here. Use this in your music JSON files.");
    lsVar("BeatPulseSpeedMult", &LevelStatus::beatPulseSpeedMult,
        "Gets how fast the polygon pulses with the beatpulse. This is very useful "
        "to help keep your level in sync with the music.",
        
        "Sets how fast the polygon pulses with beatpulse to `$0`.");
    lsVar("RadiusMin", &LevelStatus::radiusMin,
        "Gets the minimum radius of the polygon in a level. This is used to determine "
        "the absolute size of the polygon in the level.",
    
        "Sets the minimum radius of the polygon to `$0`. Use this to set the size of "
        "the polygon in the level, not ``BeatPulseMax``.");
    lsVar("WallSkewLeft", &LevelStatus::wallSkewLeft,
        "Gets the Y axis offset of the top left vertex in all walls.",
    
        "Sets the Y axis offset of the top left vertex to `$0` in all newly generated "
        "walls. If you would like to have more individual control of the wall vertices, "
        "please use the custom walls system under the prefix ``cw_``.");
    lsVar("WallSkewRight", &LevelStatus::wallSkewRight,
        "Gets the Y axis offset of the top right vertex in all walls.",
    
        "Sets the Y axis offset of the top right vertex to `$0` in all newly generated "
        "walls. If you would like to have more individual control of the wall vertices, "
        "please use the custom walls system under the prefix ``cw_``.");
    lsVar("WallAngleLeft", &LevelStatus::wallAngleLeft,
        "Gets the X axis offset of the top left vertex in all walls.",
    
        "Sets the X axis offset of the top left vertex to `$0` in all newly generated "
        "walls. If you would like to have more individual control of the wall vertices, "
        "please use the custom walls system under the prefix ``cw_``.");
    lsVar("WallAngleRight", &LevelStatus::wallAngleRight,
        "Gets the X axis offset of the top right vertex in all walls.",
    
        "Sets the X axis offset of the top right vertex to `$0` in all newly generated "
        "walls. If you would like to have more individual control of the wall vertices, "
        "please use the custom walls system under the prefix ``cw_``.");
    lsVar("3dRequired", &LevelStatus::_3DRequired,
        "Gets whether 3D must be enabled in order to have a valid score in this level. "
        "By default, this value is ``false``.",
    
        "Sets whether 3D must be enabled to `$0` to have a valid score. Only set this "
        "to ``true`` if your level relies on 3D effects to work as intended.");
    // Commenting this one out. This property seems to have NO USE in the actual game itself.
    // lsVar("3dEffectMultiplier", &LevelStatus::_3dEffectMultiplier);
    lsVar("CameraShake", &LevelStatus::cameraShake,
        "Gets the intensity of the camera shaking in a level.",
    
        "Sets the intensity of the camera shaking in a level to `$0`. This remains "
        "permanent until you either set this to 0 or the player dies.");
    lsVar("Sides", &LevelStatus::sides,
        "Gets the current number of sides on the polygon in a level.",
    
        "Sets the current number of sides on the polygon to `$0`. This change happens "
        "immediately and previously spawned walls will not adjust to the new side count.");
    lsVar("SidesMax", &LevelStatus::sidesMax,
        "Gets the maximum range that the number of sides can possibly be at random. "
        "``enableRndSideChanges`` must be enabled for this property to have any use.",
    
        "Sets the maximum range that the number of sides can possibly be to `$0`.");
    lsVar("SidesMin", &LevelStatus::sidesMin,
        "Gets the minimum range that the number of sides can possibly be at random. "
        "``enableRndSideChanges`` must be enabled for this property to have any use.",
    
        "Sets the minimum range that the number of sides can possibly be to `$0`.");
    lsVar("SwapEnabled", &LevelStatus::swapEnabled,
        "Gets whether the swap mechanic is enabled for a level. By default, this is "
        "set to ``false``.",
    
        "Sets the swap mechanic's availability to `$0`.");
    lsVar("TutorialMode", &LevelStatus::tutorialMode,
        "Gets whether tutorial mode is enabled. In tutorial mode, players are granted "
        "invincibility from dying to walls. This mode is typically enabled whenever a "
        "pack developer needs to demonstrate a new concept to the player so that way "
        "they can easily learn the new mechanic/concept. This invincibility will not "
        "count towards invalidating a score, but it's usually not important to score "
        "on a tutorial level. By default, this is set to ``false``.",
    
        "Sets tutorial mode to `$0`. Remember, only enable this if you need to demonstrate "
        "a new concept for players to learn, or use it as a gimmick to a level.");
    lsVar("IncEnabled", &LevelStatus::incEnabled,
        "Gets whether the level can increment or not. This is Open Hexagon's way of "
        "establishing a difficulty curve in the level and set a sense of progression "
        "throughout the level. By default, this value is set to ``true``.",
    
        "Toggles level incrementation to `$0`. Only disable this if you feel like the "
        "level can not benefit from incrementing in any way.");
    lsVar("DarkenUnevenBackgroundChunk",
        &LevelStatus::darkenUnevenBackgroundChunk,
        "Gets whether the ``Nth`` panel of a polygon with ``N`` sides (assuming ``N`` "
        "is odd) will be darkened to make styles look more balanced. By default, this "
        "value is set to ``true``, but there can be styles where having this darkened "
        "panel can look very unpleasing.",
        
        "Sets the darkened panel to `$0`.");
    lsVar("CurrentIncrements", &LevelStatus::currentIncrements,
        "Gets the current amount of times the level has incremented. Very useful for "
        "keeping track of levels.",

        "Sets the current amount of times the level has incremented to `$0`. This "
        "function is utterly pointless to use unless you are tracking this variable.");

    addLuaFn("l_enableRndSideChanges", //
        [this](bool mValue) { levelStatus.rndSideChangesEnabled = mValue; })
        .arg("enabled")
        .doc(
            "Toggles random side changes to `$0`, (not) allowing sides to change "
            "between ``SidesMin`` and ``SidesMax`` inclusively every level increment.");

    // Commented out for redundancy
    
    // addLuaFn("l_darkenUnevenBackgroundChunk", //
    //     [this](
    //         bool mValue) { levelStatus.darkenUnevenBackgroundChunk = mValue; })
    //     .arg("enabled")
    //     .doc(
    //         "If `$0` is true, one of the background's chunks will be darkened "
    //         "in case there is an uneven number of sides.");

    addLuaFn("l_addTracked", //
        [this](std::string mVar, std::string mName) {
            levelStatus.trackedVariables.emplace_back(mVar, mName);
        })
        .arg("variable")
        .arg("name")
        .doc(
            "Add the variable `$0` to the list of tracked variables, with name "
            "`$1`. Tracked variables are displayed in game, below the game "
            "timer.");

    addLuaFn("l_setRotation", //
        [this](float mValue) { backgroundCamera.setRotation(mValue); })
        .arg("angle")
        .doc("Set the background camera rotation to `$0` radians.");

    addLuaFn("l_getRotation", //
        [this] { return backgroundCamera.getRotation(); })
        .doc("Return the background camera rotation, in radians.");

    addLuaFn("l_getLevelTime", //
        [this] { return status.getTimeSeconds(); })
        .doc("Get the current game timer value, in seconds.");

    addLuaFn("l_getOfficial", //
        [this] { return Config::getOfficial(); })
        .doc(
            "Return `true` if \"official mode\" is enabled, `false` "
            "otherwise.");

    // TODO: test and consider re-enabling
    /*
    addLuaFn("l_setLevel",
     [this](std::string mId)
        {
            setLevelData(assets.getLevelData(mId), true);
            stopLevelMusic();
            playLevelMusic();
        });
    */
}



void HexagonGame::initLua_StyleControl()
{
    const auto sdVar = [this](const std::string& name, auto pmd) {
        using Type = std::decay_t<decltype(styleData.*pmd)>;

        std::string getDocString = "Return the `";
        getDocString += name;
        getDocString += "` field of the style data.";

        std::string setDocString = "Set the `";
        setDocString += name;
        setDocString += "` field of the style data to `$0`.";

        addLuaFn(std::string{"s_get"} + name, //
            [this, pmd]() -> Type { return styleData.*pmd; })
            .doc(getDocString);

        addLuaFn(std::string{"s_set"} + name, //
            [this, pmd](Type mValue) { styleData.*pmd = mValue; })
            .arg("value")
            .doc(setDocString);
        ;
    };

    sdVar("HueMin", &StyleData::hueMin);
    sdVar("HueMax", &StyleData::hueMax);
    sdVar("HueInc", &StyleData::hueIncrement); // backwards-compatible
    sdVar("HueIncrement", &StyleData::hueIncrement);
    sdVar("PulseMin", &StyleData::pulseMin);
    sdVar("PulseMax", &StyleData::pulseMax);
    sdVar("PulseInc", &StyleData::pulseIncrement); // backwards-compatible
    sdVar("PulseIncrement", &StyleData::pulseIncrement);
    sdVar("HuePingPong", &StyleData::huePingPong);
    sdVar("MaxSwapTime", &StyleData::maxSwapTime);
    sdVar("3dDepth", &StyleData::_3dDepth);
    sdVar("3dSkew", &StyleData::_3dSkew);
    sdVar("3dSpacing", &StyleData::_3dSpacing);
    sdVar("3dDarkenMult", &StyleData::_3dDarkenMult);
    sdVar("3dAlphaMult", &StyleData::_3dAlphaMult);
    sdVar("3dAlphaFalloff", &StyleData::_3dAlphaFalloff);
    sdVar("3dPulseMax", &StyleData::_3dPulseMax);
    sdVar("3dPulseMin", &StyleData::_3dPulseMin);
    sdVar("3dPulseSpeed", &StyleData::_3dPulseSpeed);
    sdVar("3dPerspectiveMult", &StyleData::_3dPerspectiveMult);
    sdVar("BGTileRadius", &StyleData::bgTileRadius);
    sdVar("BGColorOffset", &StyleData::BGColorOffset);
    sdVar("BGRotationOffset", &StyleData::BGRotOff);

    addLuaFn("s_setStyle", //
        [this](std::string mId) {
            styleData = assets.getStyleData(levelData->packId, mId);
        })
        .arg("styleId")
        .doc(
            "Set the currently active style to the style with id `$0`. Styles "
            "can be defined as `.json` files in the `<pack>/Styles/` folder.");

    // backwards-compatible
    addLuaFn("s_setCameraShake", //
        [this](int mValue) { levelStatus.cameraShake = mValue; })
        .arg("value")
        .doc("Start a camera shake with intensity `$0`.");

    // backwards-compatible
    addLuaFn("s_getCameraShake", //
        [this] { return levelStatus.cameraShake; })
        .doc("Return the current camera shake intensity.");

    addLuaFn("s_setCapColorMain", //
        [this] { styleData.capColor = CapColorMode::Main{}; })
        .doc(
            "Set the color of the center polygon to match the main style "
            "color.");

    addLuaFn("s_setCapColorMainDarkened", //
        [this] { styleData.capColor = CapColorMode::MainDarkened{}; })
        .doc(
            "Set the color of the center polygon to match the main style "
            "color, darkened.");

    addLuaFn("s_setCapColorByIndex", //
        [this](
            int mIndex) { styleData.capColor = CapColorMode::ByIndex{mIndex}; })
        .arg("index")
        .doc(
            "Set the color of the center polygon to match the  style "
            "color with index `$0`.");
}

void HexagonGame::initLua_WallCreation()
{
    addLuaFn("w_wall", //
        [this](int mSide, float mThickness) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness, {getSpeedMultDM()});
            });
        })
        .arg("side")
        .arg("thickness")
        .doc(
            "Create a new wall at side `$0`, with thickness `$1`. The speed of "
            "the wall will be calculated by using the speed multiplier, "
            "adjusted for the current difficulty multiplier.");

    addLuaFn("w_wallAdj", //
        [this](int mSide, float mThickness, float mSpeedAdj) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness, mSpeedAdj * getSpeedMultDM());
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

    addLuaFn("w_wallAcc", //
        [this](int mSide, float mThickness, float mSpeedAdj,
            float mAcceleration, float mMinSpeed, float mMaxSpeed) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness,
                    {mSpeedAdj * getSpeedMultDM(),
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

    addLuaFn("w_wallHModSpeedData", //
        [this](float mHMod, int mSide, float mThickness, float mSAdj,
            float mSAcc, float mSMin, float mSMax, bool mSPingPong) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness,
                    {mSAdj * getSpeedMultDM(), mSAcc, mSMin, mSMax, mSPingPong},
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

    addLuaFn("w_wallHModCurveData", //
        [this](float mHMod, int mSide, float mThickness, float mCAdj,
            float mCAcc, float mCMin, float mCMax, bool mCPingPong) {
            timeline.append_do([=, this] {
                createWall(mSide, mThickness, {getSpeedMultDM()},
                    {mCAdj, mCAcc, mCMin, mCMax, mCPingPong}, mHMod);
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
    addLuaFn("steam_unlockAchievement", //
        [this](std::string mId) {
            if(Config::getOfficial())
            {
                steamManager.unlock_achievement(mId);
            }
        })
        .arg("achievementId")
        .doc("Unlock the Steam achievement with id `$0`.");
}

void HexagonGame::initLua_CustomWalls()
{
    addLuaFn("cw_create", //
        [this]() -> CCustomWallHandle { return cwManager.create(); })
        .doc("Create a new custom wall and return a integer handle to it.");

    addLuaFn("cw_destroy", //
        [this](CCustomWallHandle cwHandle) { cwManager.destroy(cwHandle); })
        .arg("cwHandle")
        .doc("Destroy the custom wall represented by `$0`.");

    addLuaFn("cw_setVertexPos", //
        [this](CCustomWallHandle cwHandle, int vertexIndex, float x, float y) {
            cwManager.setVertexPos(cwHandle, vertexIndex, sf::Vector2f{x, y});
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .arg("x")
        .arg("y")
        .doc(
            "Given the custom wall represented by `$0`, set the position of "
            "its vertex with index `$1` to `{$2, $3}`.");

    addLuaFn("cw_setVertexColor", //
        [this](CCustomWallHandle cwHandle, int vertexIndex, int r, int g, int b,
            int a) {
            cwManager.setVertexColor(
                cwHandle, vertexIndex, sf::Color(r, g, b, a));
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .arg("r")
        .arg("g")
        .arg("b")
        .arg("a")
        .doc(
            "Given the custom wall represented by `$0`, set the color of "
            "its vertex with index `$1` to `{$2, $3, $4, $5}`.");

    addLuaFn("cw_getVertexPos", //
        [this](CCustomWallHandle cwHandle,
            int vertexIndex) -> std::tuple<float, float> {
            const sf::Vector2f pos =
                cwManager.getVertexPos(cwHandle, vertexIndex);
            return std::tuple{pos.x, pos.y};
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .arg("x")
        .arg("y")
        .doc(
            "Given the custom wall represented by `$0`, set the position of "
            "its vertex with index `$1` to `{$2, $3}`.");

    // TODO:
    /*
    addLuaFn("cw_isOverlappingPlayer", //
        [this](CCustomWallHandle cwHandle) -> bool {
            return cwManager.isOverlappingPlayer(cwHandle);
        })
        .arg("cwHandle")
        .doc(
            "Return `true` if the custom wall represented by `$0` is "
            "overlapping the player, `false` otherwise.");
    */

    addLuaFn("cw_clear", //
        [this] { cwManager.clear(); })
        .doc("Remove all existing custom walls.");
}

void HexagonGame::initLua()
{
    initLua_Utils();
    initLua_Messages();
    initLua_MainTimeline();
    initLua_EventTimeline();
    initLua_LevelControl();
    initLua_StyleControl();
    initLua_WallCreation();
    initLua_Steam();
    initLua_CustomWalls();

    // TODO: refactor doc stuff and have a command line option to print this:
#if 0
    ssvu::lo("hg::HexagonGame::initLua") << "Printing Lua Markdown docs\n\n";
    printLuaDocs();
    std::cout << "\n\n";
    ssvu::lo("hg::HexagonGame::initLua") << "Done\n";
#endif
}

} // namespace hg

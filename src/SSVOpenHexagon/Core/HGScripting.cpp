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
            playLevelMusicAtTime(musicData.getSegment(segment));
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
            "`$1` seconds. The message will only be printed during the first "
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
        .doc("*Add to the main timeline*: wait for `$0` sixths of a second.");

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
            "*Add to the event timeline*: pause the game timer for `$0` sixths "
            "of a second.");

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
    const auto lsVar = [this](const std::string& name, auto pmd) {
        using Type = std::decay_t<decltype(levelStatus.*pmd)>;

        std::string getDocString = "Return the `";
        getDocString += name;
        getDocString += "` field of the level status.";

        std::string setDocString = "Set the `";
        setDocString += name;
        setDocString += "` field of the level status to `$0`";

        addLuaFn(std::string{"l_get"} + name, //
            [this, pmd]() -> Type { return levelStatus.*pmd; })
            .doc(getDocString);

        addLuaFn(std::string{"l_set"} + name, //
            [this, pmd](Type mValue) { levelStatus.*pmd = mValue; })
            .arg("value")
            .doc(setDocString);
    };

    lsVar("SpeedMult", &LevelStatus::speedMult);
    lsVar("SpeedInc", &LevelStatus::speedInc);
    lsVar("SpeedMax", &LevelStatus::speedMax);
    lsVar("RotationSpeed", &LevelStatus::rotationSpeed);
    lsVar("RotationSpeedInc", &LevelStatus::rotationSpeedInc);
    lsVar("RotationSpeedMax", &LevelStatus::rotationSpeedMax);
    lsVar("DelayMult", &LevelStatus::delayMult);
    lsVar("DelayInc", &LevelStatus::delayInc);
    lsVar("DelayMin", &LevelStatus::delayMin);
    lsVar("DelayMax", &LevelStatus::delayMax);
    lsVar("FastSpin", &LevelStatus::fastSpin);
    lsVar("IncTime", &LevelStatus::incTime);
    lsVar("PulseMin", &LevelStatus::pulseMin);
    lsVar("PulseMax", &LevelStatus::pulseMax);
    lsVar("PulseSpeed", &LevelStatus::pulseSpeed);
    lsVar("PulseSpeedR", &LevelStatus::pulseSpeedR);
    lsVar("PulseDelayMax", &LevelStatus::pulseDelayMax);
    lsVar("PulseDelayHalfMax", &LevelStatus::pulseDelayHalfMax);
    lsVar("BeatPulseMax", &LevelStatus::beatPulseMax);
    lsVar("BeatPulseDelayMax", &LevelStatus::beatPulseDelayMax);
    lsVar("RadiusMin", &LevelStatus::radiusMin);
    lsVar("WallSkewLeft", &LevelStatus::wallSkewLeft);
    lsVar("WallSkewRight", &LevelStatus::wallSkewRight);
    lsVar("WallAngleLeft", &LevelStatus::wallAngleLeft);
    lsVar("WallAngleRight", &LevelStatus::wallAngleRight);
    lsVar("3dEffectMultiplier", &LevelStatus::_3dEffectMultiplier);
    lsVar("CameraShake", &LevelStatus::cameraShake);
    lsVar("Sides", &LevelStatus::sides);
    lsVar("SidesMax", &LevelStatus::sidesMax);
    lsVar("SidesMin", &LevelStatus::sidesMin);
    lsVar("SwapEnabled", &LevelStatus::swapEnabled);
    lsVar("TutorialMode", &LevelStatus::tutorialMode);
    lsVar("IncEnabled", &LevelStatus::incEnabled);
    lsVar("DarkenUnevenBackgroundChunk",
        &LevelStatus::darkenUnevenBackgroundChunk);
    lsVar("CurrentIncrements", &LevelStatus::currentIncrements);

    addLuaFn("l_enableRndSideChanges", //
        [this](bool mValue) { levelStatus.rndSideChangesEnabled = mValue; })
        .arg("enabled")
        .doc("Set random side changes to `$0`.");

    addLuaFn("l_darkenUnevenBackgroundChunk", //
        [this](
            bool mValue) { levelStatus.darkenUnevenBackgroundChunk = mValue; })
        .arg("enabled")
        .doc(
            "If `$0` is true, one of the background's chunks will be darkened "
            "in case there is an uneven number of sides.");

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
        setDocString += "` field of the style data to `$0`";

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

    // TODO: refactor this doc stuff and have a command line option to print
    // this:
    /*
    ssvu::lo("hg::HexagonGame::initLua") << "Printing Lua Markdown docs\n\n";
    printLuaDocs();
    std::cout << "\n\n";
    ssvu::lo("hg::HexagonGame::initLua") << "Done\n";
    */
}

} // namespace hg

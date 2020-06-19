// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{

void HexagonGame::initLua_Utils()
{
    lua.writeVariable("u_log", [=](string mLog) { lo("lua") << mLog << "\n"; });
    lua.writeVariable("u_execScript", [=](string mName) {
        runLuaFile(levelData->packPath + "Scripts/" + mName);
    });
    lua.writeVariable(
        "u_playSound", [=](string mId) { assets.playSound(mId); });
    lua.writeVariable("u_setMusic", [=](string mId) {
        musicData = assets.getMusicData(mId);
        musicData.firstPlay = true;
        stopLevelMusic();
        playLevelMusic();
    });
    lua.writeVariable("u_setMusicSegment", [=](string mId, int segment) {
        musicData = assets.getMusicData(mId);
        stopLevelMusic();
        playLevelMusicAtTime(musicData.getSegment[segment]);
    });
    lua.writeVariable("u_setMusicSeconds", [=](string mId, float mTime) {
        musicData = assets.getMusicData(mId);
        stopLevelMusic();
        playLevelMusicAtTime(mTime);
    });
    lua.writeVariable("u_isKeyPressed",
        [=](int mKey) { return window.getInputState()[KKey(mKey)]; });

    lua.writeVariable(
        "u_getPlayerAngle", [=] { return player.getPlayerAngle(); });
    lua.writeVariable("u_setPlayerAngle",
        [=](float newAng) { player.setPlayerAngle(newAng); });

    lua.writeVariable("u_isMouseButtonPressed",
        [=](int mKey) { return window.getInputState()[MBtn(mKey)]; });

    lua.writeVariable("u_isFastSpinning", [=] { return status.fastSpin > 0; });
    lua.writeVariable("u_forceIncrement", [=] { incrementDifficulty(); });
    lua.writeVariable(
        "u_kill", [=] { timeline.append<Do>([=] { death(true); }); });
    lua.writeVariable(
        "u_eventKill", [=] { eventTimeline.append<Do>([=] { death(true); }); });
    lua.writeVariable("u_getDifficultyMult", [=] { return difficultyMult; });
    lua.writeVariable("u_getSpeedMultDM", [=] { return getSpeedMultDM(); });
    lua.writeVariable("u_getDelayMultDM", [=] { return getDelayMultDM(); });
}

void HexagonGame::initLua_Messages()
{
    lua.writeVariable("m_messageAdd", [=](string mMsg, float mDuration) {
        eventTimeline.append<Do>([=] {
            if(firstPlay && Config::getShowMessages())
            {
                addMessage(mMsg, mDuration);
            }
        });
    });

    lua.writeVariable(
        "m_messageAddImportant", [=](string mMsg, float mDuration) {
            eventTimeline.append<Do>([=] {
                if(Config::getShowMessages())
                {
                    addMessage(mMsg, mDuration);
                }
            });
        });

    lua.writeVariable("m_clearMessages", [=] { clearMessages();});
}

void HexagonGame::initLua_MainTimeline()
{
    lua.writeVariable(
        "t_wait", [=](float mDuration) { timeline.append<Wait>(mDuration); });

    lua.writeVariable("t_waitS", [=](float mDuration) {
        timeline.append<Wait>(ssvu::getSecondsToFT(mDuration));
    });

    lua.writeVariable("t_waitUntilS", [=](float mDuration) {
        timeline.append<Wait>(10);
        timeline.append<Do>([=] {
            if(status.currentTime < mDuration)
            {
                timeline.jumpTo(timeline.getCurrentIndex() - 2);
            }
        });
    });
}

void HexagonGame::initLua_EventTimeline()
{
    lua.writeVariable("e_eventStopTime", [=](float mDuration) {
        eventTimeline.append<Do>([=] { status.timeStop = mDuration; });
    });

    lua.writeVariable("e_eventStopTimeS", [=](float mDuration) {
        eventTimeline.append<Do>(
            [=] { status.timeStop = ssvu::getSecondsToFT(mDuration); });
    });

    lua.writeVariable("e_eventWait",
        [=](float mDuration) { eventTimeline.append<Wait>(mDuration); });

    lua.writeVariable("e_eventWaitS", [=](float mDuration) {
        eventTimeline.append<Wait>(ssvu::getSecondsToFT(mDuration));
    });

    lua.writeVariable("e_eventWaitUntilS", [=](float mDuration) {
        eventTimeline.append<Wait>(10);
        eventTimeline.append<Do>([=] {
            if(status.currentTime < mDuration)
            {
                eventTimeline.jumpTo(eventTimeline.getCurrentIndex() - 2);
            }
        });
    });
}

void HexagonGame::initLua_LevelControl()
{

    const auto lsVar = [this](const std::string& name, auto pmd) {
        using Type = std::decay_t<decltype(levelStatus.*pmd)>;

        lua.writeVariable(std::string{"l_get"} + name,
            [this, pmd]() -> Type { return levelStatus.*pmd; });

        lua.writeVariable(std::string{"l_set"} + name,
            [this, pmd](Type mValue) { levelStatus.*pmd = mValue; });
    };

    lsVar("SpeedMult", &LevelStatus::speedMult);
    lsVar("SpeedInc", &LevelStatus::speedInc);
    lsVar("RotationSpeed", &LevelStatus::rotationSpeed);
    lsVar("RotationSpeedInc", &LevelStatus::rotationSpeedInc);
    lsVar("RotationSpeedMax", &LevelStatus::rotationSpeedMax);
    lsVar("DelayMult", &LevelStatus::delayMult);
    lsVar("DelayInc", &LevelStatus::delayInc);
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
    lsVar("RndSideChangesEnabled", &LevelStatus::rndSideChangesEnabled);
    lsVar("DarkenUnevenBackgroundChunk",
        &LevelStatus::darkenUnevenBackgroundChunk);
    lsVar("CurrentIncrements", &LevelStatus::currentIncrements);
    lsVar("MaxInc", &LevelStatus::maxIncrements); // backwards-compatible
    lsVar("MaxIncrements", &LevelStatus::maxIncrements);

    lua.writeVariable("l_addTracked", [=](string mVar, string mName) {
        levelStatus.trackedVariables.emplace_back(mVar, mName);
    });

    lua.writeVariable("l_setRotation",
        [=](float mValue) { backgroundCamera.setRotation(mValue); });

    lua.writeVariable(
        "l_getRotation", [=] { return backgroundCamera.getRotation(); });

    lua.writeVariable(
        "l_getLevelTime", [=] { return (float)status.currentTime; });

    lua.writeVariable("l_getOfficial", [=] { return Config::getOfficial(); });

    // TODO: test and consider re-enabling
    /*
    lua.writeVariable("l_setLevel", [=](string mId)
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

        lua.writeVariable(std::string{"s_get"} + name,
            [this, pmd]() -> Type { return styleData.*pmd; });

        lua.writeVariable(std::string{"s_set"} + name,
            [this, pmd](Type mValue) { styleData.*pmd = mValue; });
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

    lua.writeVariable("s_setStyle",
        [=](string mId) { styleData = assets.getStyleData(mId); });

    // backwards-compatible
    lua.writeVariable("s_setCameraShake",
        [=](int mValue) { levelStatus.cameraShake = mValue; });

    // backwards-compatible
    lua.writeVariable(
        "s_getCameraShake", [=] { return levelStatus.cameraShake; });
}

void HexagonGame::initLua_WallCreation()
{
    lua.writeVariable("w_wall", [=](int mSide, float mThickness) {
        timeline.append<Do>(
            [=] { createWall(mSide, mThickness, {getSpeedMultDM()}); });
    });

    lua.writeVariable(
        "w_wallAdj", [=](int mSide, float mThickness, float mSpeedAdj) {
            timeline.append<Do>([=] {
                createWall(mSide, mThickness, mSpeedAdj * getSpeedMultDM());
            });
        });

    lua.writeVariable("w_wallAcc",
        [=](int mSide, float mThickness, float mSpeedAdj, float mAcceleration,
            float mMinSpeed, float mMaxSpeed) {
            timeline.append<Do>([=] {
                createWall(mSide, mThickness,
                    {mSpeedAdj * getSpeedMultDM(), mAcceleration,
                        mMinSpeed * getSpeedMultDM(),
                        mMaxSpeed * getSpeedMultDM()});
            });
        });

    lua.writeVariable("w_wallHModSpeedData",
        [=](float mHMod, int mSide, float mThickness, float mSAdj, float mSAcc,
            float mSMin, float mSMax, bool mSPingPong) {
            timeline.append<Do>([=] {
                createWall(mSide, mThickness,
                    {mSAdj * getSpeedMultDM(), mSAcc, mSMin, mSMax, mSPingPong},
                    mHMod);
            });
        });

    lua.writeVariable("w_wallHModCurveData",
        [=](float mHMod, int mSide, float mThickness, float mCAdj, float mCAcc,
            float mCMin, float mCMax, bool mCPingPong) {
            timeline.append<Do>([=] {
                createWall(mSide, mThickness, {getSpeedMultDM()},
                    {mCAdj, mCAcc, mCMin, mCMax, mCPingPong}, mHMod);
            });
        });
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
}

} // namespace hg

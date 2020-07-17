// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"

#include <SSVStart/Utils/Vector2.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu;
using namespace hg::Utils;

namespace hg
{

void HexagonGame::update(ssvu::FT mFT)
{
    mFT *= Config::getTimescale();

    // TODO: refactor to avoid repetition, and truncate floating point number
    // TODO: also show best record (here) and last run + best record (in menu)
    steamManager.set_rich_presence_in_game(
        levelData->name, status.getTimeSeconds());
    steamManager.run_callbacks();

    discordManager.set_rich_presence_in_game(
        levelData->name, status.getTimeSeconds());
    discordManager.run_callbacks();

    hg::Joystick::update();

    updateText();
    updateFlash(mFT);
    effectTimelineManager.update(mFT);

    // Joystick support
    const bool jCW = hg::Joystick::rightPressed();
    const bool jCCW = hg::Joystick::leftPressed();

    if(!status.started && (!Config::getRotateToStart() || inputImplCCW ||
                              inputImplCW || inputImplBothCWCCW || jCW || jCCW))
    {
        status.start();
        messageText.setString("");
        assets.playSound("go.ogg");
        assets.musicPlayer.resume();
        if(Config::getOfficial())
        {
            fpsWatcher.enable();
        }
    }

    // Naive touch controls
    for(const auto& p : window.getFingerDownPositions())
    {
        if(p.x < window.getWidth() / 2.f)
        {
            inputImplCCW = 1;
        }
        else
        {
            inputImplCW = 1;
        }
    }

    if(inputImplCW && !inputImplCCW)
    {
        inputMovement = 1;
    }
    else if(!inputImplCW && inputImplCCW)
    {
        inputMovement = -1;
    }
    else if(inputImplCW && inputImplCCW)
    {
        if(!inputImplBothCWCCW)
        {
            if(inputMovement == 1 && inputImplLastMovement == 1)
            {
                inputMovement = -1;
            }
            else if(inputMovement == -1 && inputImplLastMovement == -1)
            {
                inputMovement = 1;
            }
        }
    }
    else
    {
        inputMovement = 0;

        // Joystick support
        {
            if(jCW && !jCCW)
            {
                inputMovement = 1;
            }
            else if(!jCW && jCCW)
            {
                inputMovement = -1;
            }
            else if(jCW && jCCW)
            {
                if(!inputImplBothCWCCW)
                {
                    if(inputMovement == 1 && inputImplLastMovement == 1)
                    {
                        inputMovement = -1;
                    }
                    else if(inputMovement == -1 && inputImplLastMovement == -1)
                    {
                        inputMovement = 1;
                    }
                }
            }
            else
            {
                inputMovement = 0;
            }
        }
    }

    updateKeyIcons();

    // Joystick support
    if(hg::Joystick::selectRisingEdge())
    {
        goToMenu();
    }
    else if(hg::Joystick::startRisingEdge())
    {
        status.mustRestart = true;
    }

    if(status.started)
    {
        if(!assets.pIsLocal() && Config::isEligibleForScore())
        {
            assets.playedSeconds += ssvu::getFTToSeconds(mFT);
            if(assets.playedSeconds >= 60.f)
            {
                assets.playedSeconds = 0;
                Online::trySendMinutePlayed();
            }
        }

        if(!status.hasDied)
        {
            player.update(*this, mFT);

            for(CWall& w : walls)
            {
                w.update(*this, centerPos, mFT);
            }

            ssvu::eraseRemoveIf(walls, [](const auto& w) { return w.killed; });
            cwManager.cleanup();

            updateEvents(mFT);
            status.accumulateFrametime(mFT);
            updateIncrement();

            if(mustChangeSides && walls.empty())
            {
                sideChange(
                    getRndI(levelStatus.sidesMin, levelStatus.sidesMax + 1));
            }

            updateLevel(mFT);

            if(Config::getBeatPulse())
            {
                updateBeatPulse(mFT);
            }

            if(Config::getPulse())
            {
                updatePulse(mFT);
            }

            if(!Config::getBlackAndWhite())
            {
                styleData.update(mFT, pow(difficultyMult, 0.8f));
            }
        }
        else
        {
            levelStatus.rotationSpeed *= 0.99f;
        }

        if(Config::get3D())
        {
            update3D(mFT);
        }
        if(!Config::getNoRotation())
        {
            updateRotation(mFT);
        }
    }

    overlayCamera.update(mFT);
    backgroundCamera.update(mFT);

    if(status.started)
    {
        if(status.mustRestart)
        {
            fpsWatcher.disable();
            changeLevel(getPackId(), restartId, restartFirstTime);
            if(!assets.pIsLocal() && Config::isEligibleForScore())
            {
                Online::trySendRestart();
            }
        }
        if(!status.scoreInvalid && Config::getOfficial() &&
            fpsWatcher.isLimitReached())
        {
            invalidateScore("PERFORMANCE ISSUES");
        }
        else if(!status.scoreInvalid && !Config::get3D() &&
                levelStatus._3DRequired)
        {
            invalidateScore("3D REQUIRED");
        }
        fpsWatcher.update();
    }
}
void HexagonGame::updateEvents(ssvu::FT)
{
    if(const auto o =
            eventTimelineRunner.update(eventTimeline, status.getTimeTP());
        o == hg::Utils::timeline2_runner::outcome::finished)
    {
        eventTimeline.clear();
        eventTimelineRunner = {};
    }

    if(const auto o = messageTimelineRunner.update(
           messageTimeline, status.getCurrentTP());
        o == hg::Utils::timeline2_runner::outcome::finished)
    {
        messageTimeline.clear();
        messageTimelineRunner = {};
    }
}
void HexagonGame::updateIncrement()
{
    if(!levelStatus.incEnabled)
    {
        return;
    }

    if(status.getIncrementTimeSeconds() < levelStatus.incTime)
    {
        return;
    }

    ++levelStatus.currentIncrements;
    incrementDifficulty();
    status.resetIncrementTime();
    mustChangeSides = true;
}
void HexagonGame::updateLevel(ssvu::FT mFT)
{
    if(status.isTimePaused())
    {
        return;
    }

    runLuaFunction<float>("onUpdate", mFT);

    const auto o = timelineRunner.update(timeline, status.getTimeTP());

    if(o == hg::Utils::timeline2_runner::outcome::finished && !mustChangeSides)
    {
        timeline.clear();
        runLuaFunction<void>("onStep");
        timelineRunner = {};
    }
}
void HexagonGame::updatePulse(ssvu::FT mFT)
{
    if(status.pulseDelay <= 0 && status.pulseDelayHalf <= 0)
    {
        float pulseAdd{status.pulseDirection > 0 ? levelStatus.pulseSpeed
                                                 : -levelStatus.pulseSpeedR};
        float pulseLimit{status.pulseDirection > 0 ? levelStatus.pulseMax
                                                   : levelStatus.pulseMin};

        status.pulse += pulseAdd * mFT * getMusicDMSyncFactor();
        if((status.pulseDirection > 0 && status.pulse >= pulseLimit) ||
            (status.pulseDirection < 0 && status.pulse <= pulseLimit))
        {
            status.pulse = pulseLimit;
            status.pulseDirection *= -1;
            status.pulseDelayHalf = levelStatus.pulseDelayHalfMax;
            if(status.pulseDirection < 0)
            {
                status.pulseDelay = levelStatus.pulseDelayMax;
            }
        }
    }

    status.pulseDelay -= mFT;
    status.pulseDelayHalf -= mFT;

    float p{status.pulse / levelStatus.pulseMin};
    float rotation{backgroundCamera.getRotation()};
    backgroundCamera.setView({ssvs::zeroVec2f,
        {(Config::getWidth() * Config::getZoomFactor()) * p,
            (Config::getHeight() * Config::getZoomFactor()) * p}});
    backgroundCamera.setRotation(rotation);
}
void HexagonGame::updateBeatPulse(ssvu::FT mFT)
{
    if(status.beatPulseDelay <= 0)
    {
        status.beatPulse = levelStatus.beatPulseMax;
        status.beatPulseDelay = levelStatus.beatPulseDelayMax;
    }
    else
    {
        status.beatPulseDelay -= 1 * mFT * getMusicDMSyncFactor();
    }

    if(status.beatPulse > 0)
    {
        status.beatPulse -= (2.f * mFT * getMusicDMSyncFactor()) *
                            levelStatus.beatPulseSpeedMult;
    }

    float radiusMin{Config::getBeatPulse() ? levelStatus.radiusMin : 75};
    status.radius =
        radiusMin * (status.pulse / levelStatus.pulseMin) + status.beatPulse;
}
void HexagonGame::updateRotation(ssvu::FT mFT)
{
    auto nextRotation(getRotationSpeed() * 10.f);
    if(status.fastSpin > 0)
    {
        nextRotation +=
            abs((getSmootherStep(0, levelStatus.fastSpin, status.fastSpin) /
                    3.5f) *
                17.f) *
            getSign(nextRotation);
        status.fastSpin -= mFT;
    }

    backgroundCamera.turn(nextRotation);
}
void HexagonGame::updateFlash(ssvu::FT mFT)
{
    if(status.flashEffect > 0)
    {
        status.flashEffect -= 3 * mFT;
    }
    status.flashEffect = getClamped(status.flashEffect, 0.f, 255.f);
    for(auto i(0u); i < 4; ++i)
    {
        flashPolygon[i].color.a = status.flashEffect;
    }
}
void HexagonGame::update3D(ssvu::FT mFT)
{
    status.pulse3D += styleData._3dPulseSpeed * status.pulse3DDirection * mFT;
    if(status.pulse3D > styleData._3dPulseMax)
    {
        status.pulse3DDirection = -1;
    }
    else if(status.pulse3D < styleData._3dPulseMin)
    {
        status.pulse3DDirection = 1;
    }
}

} // namespace hg

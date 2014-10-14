// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtils/SSVUtils.hpp>
#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Global/Common.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu;
using namespace sses;
using namespace hg::Utils;

namespace hg
{
	void HexagonGame::update(FT mFT)
	{
		if(inputImplCW && !inputImplCCW) inputMovement = 1;
		else if(!inputImplCW && inputImplCCW) inputMovement = -1;
		else if(inputImplCW && inputImplCCW)
		{
			if(!inputImplBothCWCCW)
			{
				if(inputMovement == 1 && inputImplLastMovement == 1) inputMovement = -1;
				else if(inputMovement == -1 && inputImplLastMovement == -1) inputMovement = 1;
			}
		}
		else inputMovement = 0;

		if(!assets.pIsLocal() && Config::isEligibleForScore())
		{
			assets.playedSeconds += ssvu::getFTToSeconds(mFT);
			if(assets.playedSeconds >= 60.f)
			{
				assets.playedSeconds = 0;
				Online::trySendMinutePlayed();
			}
		}

		updateFlash(mFT);
		effectTimelineManager.update(mFT);

		if(!status.hasDied)
		{
			manager.update(mFT);
			updateEvents(mFT);
			updateTimeStop(mFT);
			updateIncrement();
			if(mustChangeSides && !manager.hasEntity(HGGroup::Wall)) sideChange(getRnd(levelStatus.sidesMin, levelStatus.sidesMax + 1));
			updateLevel(mFT);
			if(Config::getBeatPulse()) updateBeatPulse(mFT);
			if(Config::getPulse()) updatePulse(mFT);
			if(!Config::getBlackAndWhite()) styleData.update(mFT, pow(difficultyMult, 0.8f));
		}
		else levelStatus.rotationSpeed *= 0.99f;

		if(Config::get3D()) update3D(mFT);
		if(!Config::getNoRotation()) updateRotation(mFT);

		overlayCamera.update(mFT);
		backgroundCamera.update(mFT);
		for(auto& c : depthCameras) c.update(mFT);

		if(status.mustRestart)
		{
			changeLevel(restartId, restartFirstTime);
			if(!assets.pIsLocal() && Config::isEligibleForScore()) { Online::trySendRestart(); }
		}
		if(!status.scoreInvalid && Config::getOfficial() && fpsWatcher.isLimitReached()) invalidateScore();

		fpsWatcher.update();
	}
	void HexagonGame::updateEvents(FT mFT)
	{
		eventTimeline.update(mFT);
		if(eventTimeline.isFinished()) { eventTimeline.clear(); eventTimeline.reset(); }

		messageTimeline.update(mFT);
		if(messageTimeline.isFinished()) { messageTimeline.clear(); messageTimeline.reset(); }
	}
	void HexagonGame::updateTimeStop(FT mFT)
	{
		if(status.timeStop <= 0)
		{
			status.currentTime += ssvu::getFTToSeconds(mFT);
			status.incrementTime += ssvu::getFTToSeconds(mFT);
		}
		else status.timeStop -= mFT;
	}
	void HexagonGame::updateIncrement()
	{
		if(!levelStatus.incEnabled) return;
		if(status.incrementTime < levelStatus.incTime) return;

		if(!levelStatus.shouldIncrement())
		{
			++levelStatus.currentIncrements;
			incrementDifficulty();
		}

		status.incrementTime = 0;

		mustChangeSides = true;
	}
	void HexagonGame::updateLevel(FT mFT)
	{
		if(status.timeStop > 0) return;

		runLuaFunction<float>("onUpdate", mFT);
		timeline.update(mFT);

		if(timeline.isFinished() && !mustChangeSides)
		{
			timeline.clear();
			runLuaFunction<void>("onStep");
			timeline.reset();
		}
	}
	void HexagonGame::updatePulse(FT mFT)
	{
		if(status.pulseDelay <= 0 && status.pulseDelayHalf <= 0)
		{
			float pulseAdd{status.pulseDirection > 0 ? levelStatus.pulseSpeed : -levelStatus.pulseSpeedR};
			float pulseLimit{status.pulseDirection > 0 ? levelStatus.pulseMax : levelStatus.pulseMin};

			status.pulse += pulseAdd * mFT * getMusicDMSyncFactor();
			if((status.pulseDirection > 0 && status.pulse >= pulseLimit) || (status.pulseDirection < 0 && status.pulse <= pulseLimit))
			{
				status.pulse = pulseLimit;
				status.pulseDirection *= -1;
				status.pulseDelayHalf = levelStatus.pulseDelayHalfMax;
				if(status.pulseDirection < 0) status.pulseDelay = levelStatus.pulseDelayMax;
			}
		}

		status.pulseDelay -= mFT;
		status.pulseDelayHalf -= mFT;

		float p{status.pulse / levelStatus.pulseMin}, rotation{backgroundCamera.getRotation()};
		backgroundCamera.setView({ssvs::zeroVec2f, {(Config::getWidth() * Config::getZoomFactor()) * p, (Config::getHeight() * Config::getZoomFactor()) * p}});
		backgroundCamera.setRotation(rotation);
	}
	void HexagonGame::updateBeatPulse(FT mFT)
	{
		if(status.beatPulseDelay <= 0)
		{
			status.beatPulse = levelStatus.beatPulseMax;
			status.beatPulseDelay = levelStatus.beatPulseDelayMax;
		}
		else status.beatPulseDelay -= 1 * mFT * getMusicDMSyncFactor();

		if(status.beatPulse > 0) status.beatPulse -= 2.f * mFT * getMusicDMSyncFactor();

		float radiusMin{Config::getBeatPulse() ? levelStatus.radiusMin : 75};
		status.radius = radiusMin * (status.pulse / levelStatus.pulseMin) + status.beatPulse;
	}
	void HexagonGame::updateRotation(FT mFT)
	{
		auto nextRotation(getRotationSpeed() * 10.f);
		if(status.fastSpin > 0)
		{
			nextRotation += abs((getSmootherStep(0, levelStatus.fastSpin, status.fastSpin) / 3.5f) * 17.f) * getSign(nextRotation);
			status.fastSpin -= mFT;
		}

		backgroundCamera.turn(nextRotation);
	}
	void HexagonGame::updateFlash(FT mFT)
	{
		if(status.flashEffect > 0) status.flashEffect -= 3 * mFT;
		status.flashEffect = getClamped(status.flashEffect, 0.f, 255.f);
		for(auto i(0u); i < 4; ++i) flashPolygon[i].color.a = status.flashEffect;
	}
	void HexagonGame::update3D(FT mFT)
	{
		status.pulse3D += styleData._3dPulseSpeed * status.pulse3DDirection * mFT;
		if(status.pulse3D > styleData._3dPulseMax) status.pulse3DDirection = -1;
		else if(status.pulse3D < styleData._3dPulseMin) status.pulse3DDirection = 1;
	}
}

// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Global/Groups.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvu;
using namespace sses;
using namespace hg::Utils;

namespace hg
{
	void HexagonGame::update(float mFrameTime)
	{
		if(!assets.pIsLocal() && Config::isEligibleForScore())
		{
			assets.playedSeconds += mFrameTime / 60.f;
			if(assets.playedSeconds >= 60.f)
			{
				assets.playedSeconds = 0;
				Online::trySendMinutePlayed();
			}
		}

		updateFlash(mFrameTime);
		effectTimelineManager.update(mFrameTime);

		if(!status.hasDied)
		{
			manager.update(mFrameTime);
			updateEvents(mFrameTime);
			updateTimeStop(mFrameTime);
			updateIncrement();
			if(mustChangeSides && !manager.hasEntity(HGGroup::Wall)) sideChange(getRnd(levelStatus.sidesMin, levelStatus.sidesMax + 1));
			updateLevel(mFrameTime);
			if(Config::getBeatPulse()) updateBeatPulse(mFrameTime);
			if(Config::getPulse()) updatePulse(mFrameTime);
			if(!Config::getBlackAndWhite()) styleData.update(mFrameTime, pow(difficultyMult, 0.8f));
		}
		else levelStatus.rotationSpeed *= 0.99f;

		if(Config::get3D()) update3D(mFrameTime);
		if(!Config::getNoRotation()) updateRotation(mFrameTime);

		overlayCamera.update(mFrameTime);
		backgroundCamera.update(mFrameTime);
		for(auto& c : depthCameras) c.update(mFrameTime);

		if(status.mustRestart)
		{
			changeLevel(restartId, restartFirstTime);
			if(!assets.pIsLocal() && Config::isEligibleForScore()) { Online::trySendRestart(); }
		}
		if(!status.scoreInvalid && Config::getOfficial() && fpsWatcher.isLimitReached()) invalidateScore();

		fpsWatcher.update();
	}
	void HexagonGame::updateEvents(float mFrameTime)
	{
		eventTimeline.update(mFrameTime);
		if(eventTimeline.isFinished()) { eventTimeline.clear(); eventTimeline.reset(); }

		messageTimeline.update(mFrameTime);
		if(messageTimeline.isFinished()) { messageTimeline.clear(); messageTimeline.reset(); }
	}
	void HexagonGame::updateTimeStop(float mFrameTime)
	{
		if(status.timeStop <= 0)
		{
			status.currentTime += mFrameTime / 60.f;
			status.incrementTime += mFrameTime / 60.f;
		}
		else status.timeStop -= mFrameTime;
	}
	void HexagonGame::updateIncrement()
	{
		if(!levelStatus.incEnabled) return;
		if(status.incrementTime < levelStatus.incTime) return;

		status.incrementTime = 0;
		incrementDifficulty();

		mustChangeSides = true;
	}
	void HexagonGame::updateLevel(float mFrameTime)
	{
		if(status.timeStop > 0) return;

		runLuaFunction<float>("onUpdate", mFrameTime);
		timeline.update(mFrameTime);

		if(timeline.isFinished() && !mustChangeSides)
		{
			timeline.clear();
			runLuaFunction<void>("onStep");
			timeline.reset();
		}
	}
	void HexagonGame::updatePulse(float mFrameTime)
	{
		if(status.pulseDelay <= 0 && status.pulseDelayHalf <= 0)
		{
			float pulseAdd{status.pulseDirection > 0 ? levelStatus.pulseSpeed : -levelStatus.pulseSpeedR};
			float pulseLimit{status.pulseDirection > 0 ? levelStatus.pulseMax : levelStatus.pulseMin};

			status.pulse += pulseAdd * mFrameTime * getMusicDMSyncFactor();
			if((status.pulseDirection > 0 && status.pulse >= pulseLimit) || (status.pulseDirection < 0 && status.pulse <= pulseLimit))
			{
				status.pulse = pulseLimit;
				status.pulseDirection *= -1;
				status.pulseDelayHalf = levelStatus.pulseDelayHalfMax;
				if(status.pulseDirection < 0) status.pulseDelay = levelStatus.pulseDelayMax;
			}
		}

		status.pulseDelay -= mFrameTime;
		status.pulseDelayHalf -= mFrameTime;

		float p{status.pulse / levelStatus.pulseMin}, rotation{backgroundCamera.getRotation()};
		backgroundCamera.setView({{0, 0}, {(Config::getWidth() * Config::getZoomFactor()) * p, (Config::getHeight() * Config::getZoomFactor()) * p}});
		backgroundCamera.setRotation(rotation);
	}
	void HexagonGame::updateBeatPulse(float mFrameTime)
	{
		if(status.beatPulseDelay <= 0)
		{
			status.beatPulse = levelStatus.beatPulseMax;
			status.beatPulseDelay = levelStatus.beatPulseDelayMax;
		}
		else status.beatPulseDelay -= 1 * mFrameTime * getMusicDMSyncFactor();

		if(status.beatPulse > 0) status.beatPulse -= 2.f * mFrameTime * getMusicDMSyncFactor();

		float radiusMin{Config::getBeatPulse() ? levelStatus.radiusMin : 75};
		status.radius = radiusMin * (status.pulse / levelStatus.pulseMin) + status.beatPulse;
	}
	void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation(getRotationSpeed() * 10.f);
		if(status.fastSpin > 0)
		{
			nextRotation += abs((getSmootherStep(0, levelStatus.fastSpin, status.fastSpin) / 3.5f) * 17.f) * getSign(nextRotation);
			status.fastSpin -= mFrameTime;
		}

		backgroundCamera.turn(nextRotation);
	}
	void HexagonGame::updateFlash(float mFrameTime)
	{
		if(status.flashEffect > 0) status.flashEffect -= 3 * mFrameTime;
		status.flashEffect = getClamped(status.flashEffect, 0.f, 255.f);
		for(int i{0}; i < 4; ++i) flashPolygon[i].color.a = status.flashEffect;
	}
	void HexagonGame::update3D(float mFrameTime)
	{
		status.pulse3D += styleData._3dPulseSpeed * status.pulse3DDirection * mFrameTime;
		if(status.pulse3D > styleData._3dPulseMax) status.pulse3DDirection = -1;
		else if(status.pulse3D < styleData._3dPulseMin) status.pulse3DDirection = 1;
	}
}

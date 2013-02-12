#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <SFML/Audio.hpp>
#include <SSVStart.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Data/StyleData.h"
#include "Global/Assets.h"
#include "Global/Config.h"
#include "Global/Factory.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace sses;

namespace hg
{
	void HexagonGame::update(float mFrameTime)
	{
		updateFlash(mFrameTime);
		effectTimelineManager.update(mFrameTime);

		if(!status.hasDied)
		{
			manager.update(mFrameTime);
			updateEvents(mFrameTime);
			updateTimeStop(mFrameTime);
			updateIncrement();
			updateLevel(mFrameTime);
			if(getBeatPulse()) updateBeatPulse(mFrameTime);
			if(getPulse()) updatePulse(mFrameTime);
			if(!getBlackAndWhite()) styleData.update(mFrameTime);
		}
		else setRotationSpeed(getRotationSpeed() * 0.99f);

		if(!getNoRotation()) updateRotation(mFrameTime);
		if(status.mustRestart) changeLevel(restartId, restartFirstTime);
	}
	void HexagonGame::updateEvents(float mFrameTime)
	{
		for(EventData* event : eventPtrs) event->update(mFrameTime);

		if(!eventPtrQueue.empty())
		{
			eventPtrQueue.front()->update(mFrameTime);
			if(eventPtrQueue.front()->getFinished()) { delete eventPtrQueue.front(); eventPtrQueue.pop(); }
		}

		messageTimeline.update(mFrameTime);
		if(messageTimeline.isFinished()) clearAndResetTimeline(messageTimeline);

		executeEvents(levelData.getRoot()["events"], status.currentTime);
	}
	void HexagonGame::updateTimeStop(float mFrameTime)
	{
		if(status.timeStop <= 0)
		{
			status.currentTime += mFrameTime / 60.0f;
			status.incrementTime += mFrameTime / 60.0f;
		}
		else status.timeStop -= 1 * mFrameTime;
	}
	void HexagonGame::updateIncrement()
	{
		if(!status.incrementEnabled) return;
		if(status.incrementTime < levelData.getIncrementTime()) return;

		status.incrementTime = 0;
		incrementDifficulty();
	}
	void HexagonGame::updateLevel(float mFrameTime)
	{
		runLuaFunction<float>("onUpdate", mFrameTime);
		timeline.update(mFrameTime);

		if(timeline.isFinished())
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
			float pulseAdd{status.pulseDirection > 0 ? levelData.getPulseSpeed() : -levelData.getPulseSpeedR()};
			float pulseLimit{status.pulseDirection > 0 ? levelData.getPulseMax() : levelData.getPulseMin()};
			
			status.pulse += pulseAdd * mFrameTime;
			if((status.pulseDirection > 0 && status.pulse >= pulseLimit) || (status.pulseDirection < 0 && status.pulse <= pulseLimit))
			{
				status.pulse = pulseLimit;
				status.pulseDirection *= -1;
				status.pulseDelayHalf = levelData.getPulseDelayHalfMax();
				if(status.pulseDirection < 0) status.pulseDelay = levelData.getPulseDelayMax();
			}
		}

		status.pulseDelay -= 1 * mFrameTime;
		status.pulseDelayHalf -= 1 * mFrameTime;
		
		float p{status.pulse / levelData.getPulseMin()};

		float rotation{backgroundCamera.getRotation()};
		backgroundCamera.setView({{0, 0}, {(getWidth() * getZoomFactor()) * p, (getHeight() * getZoomFactor()) * p}});
		backgroundCamera.setRotation(rotation);
	}
	void HexagonGame::updateBeatPulse(float mFrameTime)
	{
		if(status.beatPulseDelay <= 0)
		{
			status.beatPulse = levelData.getBeatPulseMax();
			status.beatPulseDelay = levelData.getBeatPulseDelayMax();
		}
		else status.beatPulseDelay -= 1 * mFrameTime;

		if(status.beatPulse > 0) status.beatPulse -= 2.f * mFrameTime;

		float radiusMin{getBeatPulse() ? levelData.getRadiusMin() : 75};
		status.radius = radiusMin * (status.pulse / levelData.getPulseMin()) + status.beatPulse;
	}
	void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = abs(getRotationSpeed()) * 10 * mFrameTime;
		if(status.fastSpin > 0)
		{
			nextRotation += abs((getSmootherStep(0, levelData.getValueFloat("fast_spin"), status.fastSpin) / 3.5f) * mFrameTime * 17.0f);
			status.fastSpin -= mFrameTime;
		}

		backgroundCamera.rotate(nextRotation * getSign(getRotationSpeed()));
	}
	void HexagonGame::updateFlash(float mFrameTime)
	{
		if(status.flashEffect > 0) status.flashEffect -= 3 * mFrameTime;
		status.flashEffect = clamp(status.flashEffect, 0.f, 255.f);
		for(int i{0}; i < 4; i++) flashPolygon[i].color.a = status.flashEffect;
	}
}

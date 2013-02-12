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

		if(!hgStatus.hasDied)
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

		updateKeys();

		if(!getNoRotation()) updateRotation(mFrameTime);
		if(hgStatus.mustRestart) changeLevel(restartId, restartFirstTime);
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

		executeEvents(levelData.getRoot()["events"], hgStatus.currentTime);
	}
	void HexagonGame::updateTimeStop(float mFrameTime)
	{
		if(hgStatus.timeStop <= 0)
		{
			hgStatus.currentTime += mFrameTime / 60.0f;
			hgStatus.incrementTime += mFrameTime / 60.0f;
		}
		else hgStatus.timeStop -= 1 * mFrameTime;
	}
	void HexagonGame::updateIncrement()
	{
		if(!hgStatus.incrementEnabled) return;
		if(hgStatus.incrementTime < levelData.getIncrementTime()) return;

		hgStatus.incrementTime = 0;
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
		if(hgStatus.pulseDelay <= 0 && hgStatus.pulseDelayHalf <= 0)
		{
			float pulseAdd{hgStatus.pulseDirection > 0 ? levelData.getPulseSpeed() : -levelData.getPulseSpeedR()};
			float pulseLimit{hgStatus.pulseDirection > 0 ? levelData.getPulseMax() : levelData.getPulseMin()};
			
			hgStatus.pulse += pulseAdd * mFrameTime;
			if((hgStatus.pulseDirection > 0 && hgStatus.pulse >= pulseLimit) || (hgStatus.pulseDirection < 0 && hgStatus.pulse <= pulseLimit))
			{
				hgStatus.pulse = pulseLimit;
				hgStatus.pulseDirection *= -1;
				hgStatus.pulseDelayHalf = levelData.getPulseDelayHalfMax();
				if(hgStatus.pulseDirection < 0) hgStatus.pulseDelay = levelData.getPulseDelayMax();
			}
		}

		hgStatus.pulseDelay -= 1 * mFrameTime;
		hgStatus.pulseDelayHalf -= 1 * mFrameTime;
		
		float p{hgStatus.pulse / levelData.getPulseMin()};

		float rotation{backgroundCamera.getRotation()};
		backgroundCamera.setView({{0, 0}, {(getWidth() * getZoomFactor()) * p, (getHeight() * getZoomFactor()) * p}});
		backgroundCamera.setRotation(rotation);
	}
	void HexagonGame::updateBeatPulse(float mFrameTime)
	{
		if(hgStatus.beatPulseDelay <= 0)
		{
			hgStatus.beatPulse = levelData.getBeatPulseMax();
			hgStatus.beatPulseDelay = levelData.getBeatPulseDelayMax();
		}
		else hgStatus.beatPulseDelay -= 1 * mFrameTime;

		if(hgStatus.beatPulse > 0) hgStatus.beatPulse -= 2.f * mFrameTime;

		float radiusMin{getBeatPulse() ? levelData.getRadiusMin() : 75};
		hgStatus.radius = radiusMin * (hgStatus.pulse / levelData.getPulseMin()) + hgStatus.beatPulse;
	}
	void HexagonGame::updateKeys()
	{
		if(isKeyPressed(Keyboard::R)) hgStatus.mustRestart = true;
		if(hgStatus.hasDied && (isKeyPressed(Keyboard::Space) || isKeyPressed(Keyboard::Return))) hgStatus.mustRestart = true;
		else if(isKeyPressed(Keyboard::Escape))	goToMenu();
	}
	void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = abs(getRotationSpeed()) * 10 * mFrameTime;
		if(hgStatus.fastSpin > 0)
		{
			nextRotation += abs((getSmootherStep(0, levelData.getValueFloat("fast_spin"), hgStatus.fastSpin) / 3.5f) * mFrameTime * 17.0f);
			hgStatus.fastSpin -= mFrameTime;
		}

		backgroundCamera.rotate(nextRotation * getSign(getRotationSpeed()));
	}
	void HexagonGame::updateFlash(float mFrameTime)
	{
		if(hgStatus.flashEffect > 0) hgStatus.flashEffect -= 3 * mFrameTime;
		hgStatus.flashEffect = clamp(hgStatus.flashEffect, 0.f, 255.f);
		for(int i{0}; i < 4; i++) flashPolygon[i].color.a = hgStatus.flashEffect;
	}
}

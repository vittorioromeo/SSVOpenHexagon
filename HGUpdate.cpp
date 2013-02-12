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

		if(!hasDied)
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
		if(mustRestart) changeLevel(restartId, restartFirstTime);
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

		executeEvents(levelData.getRoot()["events"], currentTime);
	}
	void HexagonGame::updateTimeStop(float mFrameTime)
	{
		if(timeStop <= 0)
		{
			currentTime += mFrameTime / 60.0f;
			incrementTime += mFrameTime / 60.0f;
		}
		else timeStop -= 1 * mFrameTime;
	}
	void HexagonGame::updateIncrement()
	{
		if(!incrementEnabled) return;
		if(incrementTime < levelData.getIncrementTime()) return;

		incrementTime = 0;
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
		if(pulseDelay <= 0 && pulseDelayHalf <= 0)
		{
			float pulseAdd{pulseDirection > 0 ? levelData.getPulseSpeed() : -levelData.getPulseSpeedR()};
			float pulseLimit{pulseDirection > 0 ? levelData.getPulseMax() : levelData.getPulseMin()};
			
			pulse += pulseAdd * mFrameTime;
			if((pulseDirection > 0 && pulse >= pulseLimit) || (pulseDirection < 0 && pulse <= pulseLimit))
			{
				pulse = pulseLimit;
				pulseDirection *= -1;
				pulseDelayHalf = levelData.getPulseDelayHalfMax();
				if(pulseDirection < 0) pulseDelay = levelData.getPulseDelayMax();
			}
		}

		pulseDelay -= 1 * mFrameTime;
		pulseDelayHalf -= 1 * mFrameTime;
		
		float p{pulse / levelData.getPulseMin()};
		backgroundCamera.setView({{0, 0}, {(getWidth() * getZoomFactor()) * p, (getHeight() * getZoomFactor()) * p}});
	}
	void HexagonGame::updateBeatPulse(float mFrameTime)
	{
		if(beatPulseDelay <= 0)
		{
			beatPulse = levelData.getBeatPulseMax();
			beatPulseDelay = levelData.getBeatPulseDelayMax();
		}
		else beatPulseDelay -= 1 * mFrameTime;

		if(beatPulse > 0) beatPulse -= 2.f * mFrameTime;

		float radiusMin{getBeatPulse() ? levelData.getRadiusMin() : 75};
		radius = radiusMin * (pulse / levelData.getPulseMin()) + beatPulse;
	}
	void HexagonGame::updateKeys()
	{
		if(isKeyPressed(Keyboard::R)) mustRestart = true;
		if(hasDied && (isKeyPressed(Keyboard::Space) || isKeyPressed(Keyboard::Return))) mustRestart = true;
		else if(isKeyPressed(Keyboard::Escape))	goToMenu();
	}
	void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = abs(getRotationSpeed()) * 10 * mFrameTime;
		if(fastSpin > 0)
		{
			nextRotation += abs((getSmootherStep(0, levelData.getValueFloat("fast_spin"), fastSpin) / 3.5f) * mFrameTime * 17.0f);
			fastSpin -= mFrameTime;
		}

		backgroundCamera.rotate(nextRotation * getSign(getRotationSpeed()));
	}
	void HexagonGame::updateFlash(float mFrameTime)
	{
		if(flashEffect > 0) flashEffect -= 3 * mFrameTime;
		flashEffect = clamp(flashEffect, 0.f, 255.f);
		for(int i{0}; i < 4; i++) flashPolygon[i].color.a = flashEffect;
	}
}

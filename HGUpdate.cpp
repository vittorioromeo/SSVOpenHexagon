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

namespace hg
{
	void HexagonGame::update(float mFrameTime)
	{
		if(!hasDied)
		{
			manager.update(mFrameTime);

			updateEvents(mFrameTime);
			updateTimeStop(mFrameTime);
			updateIncrement();
			updateLevel(mFrameTime);
			updateRadius(mFrameTime);
			updatePulse(mFrameTime);
			if(!getBlackAndWhite()) styleData.update(mFrameTime);
		}
		else setRotationSpeed(getRotationSpeed() * 0.99f);

		updateKeys();

		if(!getNoRotation()) updateRotation(mFrameTime);

		if(mustRestart) changeLevel(restartId, restartFirstTime);
	}
	inline void HexagonGame::updateEvents(float mFrameTime)
	{
		for(EventData& event : events)  event.update(mFrameTime);
		if(!eventQueue.empty())
		{
			eventQueue.front().update(mFrameTime);
			if(eventQueue.front().getFinished()) eventQueue.pop();
		}

		messageTimeline.update(mFrameTime);
		if(messageTimeline.getFinished()) clearAndResetTimeline(messageTimeline);

		executeEvents(levelData.getRoot()["events"], currentTime);
	}
	inline void HexagonGame::updateTimeStop(float mFrameTime)
	{
		if(timeStop <= 0)
		{
			currentTime += mFrameTime / 60.0f;
			incrementTime += mFrameTime / 60.0f;
		}
		else timeStop -= 1 * mFrameTime;
	}
	inline void HexagonGame::updateIncrement()
	{
		if(!incrementEnabled) return;
		if(incrementTime < levelData.getIncrementTime()) return;

		incrementTime = 0;
		incrementDifficulty();
	}
	inline void HexagonGame::updateLevel(float mFrameTime)
	{
		runLuaFunction<float>("onUpdate", mFrameTime);
		timeline.update(mFrameTime);

		if(timeline.getFinished())
		{
			timeline.clear();
			runLuaFunction<void>("onStep");
			timeline.reset();
		}
	}
	inline void HexagonGame::updatePulse(float mFrameTime)
	{
		if(pulseDelay <= 0)
		{
			if(pulseDirection == 1)
			{
				pulse += levelData.getPulseSpeed() * mFrameTime;

				if(pulse >= levelData.getPulseMax())
				{
					pulse = levelData.getPulseMax();
					pulseDirection *= -1;

					pulseDelay = levelData.getPulseDelayMax();
				}
			}
			else if(pulseDirection == -1)
			{
				pulse -= levelData.getPulseSpeedR() * mFrameTime;

				if(pulse <= levelData.getPulseMin())
				{
					pulse = levelData.getPulseMin();
					pulseDirection *= -1;

					pulseDelay = levelData.getPulseDelayMax();
				}
			}
		}
		else pulseDelay -= 1 * mFrameTime;

		gameTexture.setView(View{Vector2f{0,0}, Vector2f{(getSizeX() * getZoomFactor()) * pulse / levelData.getPulseMin(), (getSizeY() * getZoomFactor()) * pulse / levelData.getPulseMin()}});
	}
	inline void HexagonGame::updateRadius(float mFrameTime)
	{
		if(pulseRadiusMultiplier != 0) radius = 75.f * ((pulse / levelData.getPulseMin()) * pulseRadiusMultiplier);
	}
	inline void HexagonGame::updateKeys()
	{
		if(isKeyPressed(Keyboard::R)) mustRestart = true;
		else if(isKeyPressed(Keyboard::Escape))	goToMenu();
	}
	inline void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = abs(getRotationSpeed()) * 10 * mFrameTime;
		if(fastSpin > 0)
		{
			nextRotation += (getSmootherStep(0, levelData.getValueFloat("fast_spin"), fastSpin) / 3.5f) * mFrameTime * 17.0f;
			fastSpin -= mFrameTime;
		}

		gameSprite.rotate(nextRotation * getSign(getRotationSpeed()));
	}
}

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
			if(!getBlackAndWhite()) styleData.update(mFrameTime);
		}
		else setRotationSpeed(getRotationSpeed() * 0.99f);

		updateKeys();

		if(!getNoRotation()) updateRotation(mFrameTime);
		if(!getNo3DEffects()) update3DEffects(mFrameTime);

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
		if(messageTimeline.isFinished()) clearAndResetTimeline(messageTimeline);

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
		timeline.update(mFrameTime);

		if(timeline.isFinished())
		{
			timeline.clear();
			lua.callLuaFunction<void>("onStep");
			timeline.reset();
		}
	}
	inline void HexagonGame::updateRadius(float mFrameTime)
	{
		radiusTimer += pulseSpeed * mFrameTime;
		if(radiusTimer >= 25)
		{
			radiusTimer = 0;
			radius = maxPulse;
		}

		if(radius > minPulse) radius -= pulseSpeedBackwards * mFrameTime;
	}
	inline void HexagonGame::updateKeys()
	{
		if(isKeyPressed(Keyboard::R)) mustRestart = true;
		else if(isKeyPressed(Keyboard::Escape))	goToMenu();
	}
	inline void HexagonGame::updateRotation(float mFrameTime)
	{
		auto nextRotation = getRotationSpeed() * 10 * mFrameTime;
		if(fastSpin > 0)
		{
			nextRotation += (getSmootherStep(0, 85, fastSpin) / 3.5f) * getSign(nextRotation) * mFrameTime * 17.0f;
			fastSpin -= mFrameTime;
		}

		gameSprite.rotate(nextRotation);
	}
	inline void HexagonGame::update3DEffects(float mFrameTime)
	{
		gameTexture.setView(View{Vector2f{0,0}, Vector2f{getSizeX() * getZoomFactor() * effectX, getSizeY() * getZoomFactor() * effectY}});

		if(getRnd(0, 100) > 50)
		{
			if(getRnd(0, 100) > 50)
			{
				effectX += effectXInc * getRnd(0, 300) / 20000.f * mFrameTime * get3DEffectMult();
				if(effectX < 0.85f) effectXInc = 1;
				else if(effectX > 1.15f) effectXInc = -1;
			}
			else
			{
				effectY += effectYInc * getRnd(0, 300) / 20000.f * mFrameTime * get3DEffectMult();
				if(effectY < 0.85f) effectYInc = 1;
				else if(effectY > 1.15f) effectYInc = -1;
			}
		}		
	}
}

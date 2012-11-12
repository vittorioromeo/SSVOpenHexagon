#include "PatternManager.h"
#include "HexagonGame.h"
#include "CPlayer.h"
#include "CWall.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include "SSVStart.h"
#include "SSVEntitySystem.h"
#include "Config.h"
#include "Factory.h"

namespace hg
{
	int PatternManager::getRandomSide() { return rnd(0, sides); }
	int PatternManager::getRandomDirection() { return rnd(0, 100) > 50 ? 1 : -1; }
	float PatternManager::getPerfectThickness(float mBaseThickness)
	{
		return mBaseThickness * hgPtr->speedMult * hgPtr->delayMult;
	}
	float PatternManager::getPerfectDelay(float mThickness, float mSpeed)
	{
		return mThickness / (mSpeed * hgPtr->speedMult) + ((abs(6 - sides)) * 1.25f);
	}

	PatternManager::PatternManager(HexagonGame* mHexagonGamePtr) :
		hgPtr{mHexagonGamePtr}, timeline(hgPtr->timeline), centerPos(hgPtr->centerPos), sides{hgPtr->sides} { }

	
	void PatternManager::wall(int mSide, float mThickness) { createWall(hgPtr->manager, hgPtr, centerPos, mSide, mThickness, speed, hgPtr->speedMult); }
	void PatternManager::rWall(int mSide, float mThickness) { wall(mSide, mThickness); wall(mSide + sides / 2, mThickness); }
	void PatternManager::wallExtra(int mSide, float mThickness, int mExtra)
	{
		wall(mSide, mThickness);

		int loopDir{1};
		if (mExtra < 0) loopDir = -1;

		for (int i{0}; i != mExtra; i += loopDir) wall(mSide + i + loopDir, mThickness);
	}
	void PatternManager::rWallExtra(int mSide, float mThickness, int mExtra)
	{
		rWall(mSide, mThickness);

		int loopDir{1};
		if (mExtra < 0) loopDir = -1;

		for (int i{0}; i != mExtra; i += loopDir) rWall(mSide + i + loopDir, mThickness);
	}
	void PatternManager::barrage(int mSide, float mThickness, int mNeighbors)
	{
		for(int i{mNeighbors}; i < sides - 1 - mNeighbors; i++) wall(mSide + i + 1, mThickness);
	}
	void PatternManager::barrageOnlyNeighbors(int mSide, float mThickness, int mNeighbors)
	{
		barrage(mSide, mThickness, mNeighbors);
		wall(mSide, mThickness);
	}
	void PatternManager::altBarrage(int mSide, float mThickness, int mStep)
	{
		for(int i{0}; i < sides / mStep; i++) wall(mSide + i * mStep, mThickness);
	}

	void PatternManager::alternateBarrageDiv(int mTimes, int mDiv)
	{
		float delay{getPerfectDelay(thickness, speed) * 4.6f};

		for(int i { 0 }; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ altBarrage(i, thickness, mDiv); }});
			timeline.add(new Wait{delay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 3.2f});
	}
	void PatternManager::barrageSpin(int mTimes, float mDelayMultiplier)
	{
		float delay{getPerfectDelay(thickness, speed) * 4.6f * mDelayMultiplier};
		int startSide{getRandomSide()};
		int loopDir{getRandomDirection()};

		for(int i {0}, s {0}; i < mTimes; i++, s += loopDir)
		{
			timeline.add(new Do{[=](){ barrage(startSide + s, thickness); }});
			timeline.add(new Wait{delay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 5.2f});
	}
	void PatternManager::mirrorSpin(int mTimes)
	{
		float myThickness{getPerfectThickness(baseThickness)};
		float delay{getPerfectDelay(myThickness, speed)};
		int startSide{getRandomSide()};
		int loopDir{getRandomDirection()};

		for(int i{0}, s{0}; i < mTimes; i++, s += loopDir)
		{
			timeline.add(new Do{[=]() { rWall(startSide + s, myThickness); }});
			timeline.add(new Wait{delay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 7.0f});
	}
	void PatternManager::evilRSpin(int mTimes, int mSteps)
	{
		float delay{getPerfectDelay(thickness, speed) * 4.0f};
		int startSide{getRandomSide()};
		int loopDir{getRandomDirection()};
		int currentSide{startSide};

		for(int j{0}; j < mTimes; j++)
		{
			for(int i{0}; i < mSteps; i++)
			{
				currentSide += loopDir;

				timeline.add(new Do{[=](){ rWallExtra(currentSide, thickness, loopDir); }});
				timeline.add(new Wait{delay});
			}

			loopDir *= -1;

			for(int i{0}; i < mSteps + 1; i++)
			{
				currentSide += loopDir;

				timeline.add(new Do{[=](){ rWallExtra(currentSide, thickness, loopDir); }});
				timeline.add(new Wait{delay});
			}
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 4.7f});
	}
	void PatternManager::inverseBarrage(int mTimes)
	{
		float delay{getPerfectDelay(thickness, speed) * 9.0f};
		int startSide{getRandomSide()};

		for(int i{0}; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ barrage(startSide, thickness); }});
			timeline.add(new Wait{delay});
			timeline.add(new Do{[=](){ barrage(startSide + sides / 2, thickness); }});
			timeline.add(new Wait{delay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 1.7f});
	}
	void PatternManager::rWallStrip(int mTimes)
	{
		float delay{getPerfectDelay(thickness, speed) * 2.5f};
		int startSide{getRandomSide()};

		for(int i{0}; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ rWall(startSide, thickness); }});
			timeline.add(new Wait{delay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 0.9f});
	}
}

#include <iostream>
#include <sstream>
#include <string>
#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Global/Config.h"
#include "Global/Factory.h"
#include "Utils/Utils.h"
#include "HexagonGame.h"
#include "PatternManager.h"

namespace hg
{
	void PatternManager::setAdj(float mAdjDelay, float mAdjSpeed, float mAdjThickness)
	{
		adjDelay = mAdjDelay;
		adjSpeed = mAdjSpeed;
		adjThickness = mAdjThickness;

		timeline.add(new Do{[=]
		{
			adjDelay = mAdjDelay;
			adjSpeed = mAdjSpeed;
			adjThickness = mAdjThickness;
		}});
	}
	void PatternManager::resetAdj()
	{
		timeline.add(new Do{[=]
		{
			adjDelay = 1.0f;
			adjSpeed = 1.0f;
			adjThickness = 1.0f;
		}});
	}

	int PatternManager::getRandomSide() { return getRnd(0, hgPtr->getSides()); }
	int PatternManager::getRandomDirection() { return getRnd(0, 100) > 50 ? 1 : -1; }
	float PatternManager::getPerfectThickness(float mBaseThickness)
	{
		return mBaseThickness * hgPtr->getSpeedMultiplier() * hgPtr->getDelayMultiplier();
	}
	float PatternManager::getPerfectDelay(float mThickness, float mSpeed)
	{
		return mThickness / (mSpeed * hgPtr->getSpeedMultiplier()) + ((abs(6 - hgPtr->getSides())) * 1.25f);
	}

	PatternManager::PatternManager(HexagonGame* mHexagonGamePtr) :
		hgPtr{mHexagonGamePtr}, timeline(hgPtr->timeline), centerPos(hgPtr->centerPos) { }

	
	void PatternManager::wall(int mSide, float mThickness) { createWall(hgPtr->manager, hgPtr, centerPos, mSide, mThickness * adjThickness, speed * adjSpeed, hgPtr->getSpeedMultiplier()); }
	void PatternManager::rWall(int mSide, float mThickness) { wall(mSide, mThickness); wall(mSide + hgPtr->getSides() / 2, mThickness); }
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
		for(int i{mNeighbors}; i < hgPtr->getSides() - 1 - mNeighbors; i++) wall(mSide + i + 1, mThickness);
	}
	void PatternManager::barrageOnlyNeighbors(int mSide, float mThickness, int mNeighbors)
	{
		barrage(mSide, mThickness, mNeighbors);
		wall(mSide, mThickness);
	}
	void PatternManager::altBarrage(int mSide, float mThickness, int mStep)
	{
		for(int i{0}; i < hgPtr->getSides() / mStep; i++) wall(mSide + i * mStep, mThickness);
	}

	void PatternManager::alternateWallBarrage(int mTimes, int mDiv)
	{
		float delay{getPerfectDelay(thickness, speed) * 4.6f};

		for(int i { 0 }; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ altBarrage(i, thickness, mDiv); }});
			timeline.add(new Wait{delay * adjDelay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 3.2f});
	}
	void PatternManager::barrageSpiral(int mTimes, float mDelayMultiplier)
	{
		float delay{getPerfectDelay(thickness, speed) * 4.6f * mDelayMultiplier};
		int startSide{getRandomSide()};
		int loopDir{getRandomDirection()};

		for(int i {0}, s {0}; i < mTimes; i++, s += loopDir)
		{
			timeline.add(new Do{[=](){ barrage(startSide + s, thickness); }});
			timeline.add(new Wait{delay * adjDelay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 5.2f});
	}
	void PatternManager::mirrorSpiral(int mTimes, int mExtra)
	{
		float myThickness{getPerfectThickness(baseThickness)};
		float delay{getPerfectDelay(myThickness, speed)};
		int startSide{getRandomSide()};
		int loopDir{getRandomDirection()};

		for(int i{0}, s{0}; i < mTimes; i++, s += loopDir)
		{
			timeline.add(new Do{[=]() { rWallExtra(startSide + s, myThickness, mExtra); }});
			timeline.add(new Wait{delay * adjDelay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 7.0f});
	}
	void PatternManager::extraWallVortex(int mTimes, int mSteps)
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
				timeline.add(new Wait{delay * adjDelay});
			}

			loopDir *= -1;

			for(int i{0}; i < mSteps + 1; i++)
			{
				currentSide += loopDir;

				timeline.add(new Do{[=](){ rWallExtra(currentSide, thickness, loopDir); }});
				timeline.add(new Wait{delay * adjDelay});
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
			timeline.add(new Wait{delay * adjDelay});
			timeline.add(new Do{[=](){ barrage(startSide + hgPtr->getSides() / 2, thickness); }});
			timeline.add(new Wait{delay * adjDelay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 1.7f});
	}
	void PatternManager::mirrorWallStrip(int mTimes, int mExtra)
	{
		float delay{getPerfectDelay(thickness, speed) * 2.5f};
		int startSide{getRandomSide()};

		for(int i{0}; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ rWallExtra(startSide, thickness, mExtra); }});
			timeline.add(new Wait{delay * adjDelay});
		}

		timeline.add(new Wait{getPerfectDelay(thickness, speed) * 0.9f});
	}
	void PatternManager::tunnelBarrage(int mTimes)
	{		
		float myThickness{getPerfectThickness(baseThickness)};
		float delay{getPerfectDelay(myThickness, speed) * 5.1f};
		int startSide{getRandomSide()};
		int loopDir{getRandomDirection()};

		for(int i{0}; i < mTimes; i++)
		{
			if (i < mTimes - 1) timeline.add(new Do{[=](){ wall(startSide, myThickness + (speed * hgPtr->getSpeedMultiplier()) * delay); }});
			timeline.add(new Do{[=](){ barrage(startSide + loopDir, myThickness); }});
			timeline.add(new Wait{delay * adjDelay});

			loopDir *= -1;
		}

		timeline.add(new Wait{getPerfectDelay(myThickness, speed) * 5.0f});
	}
}

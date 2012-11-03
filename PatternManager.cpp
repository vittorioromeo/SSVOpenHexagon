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

namespace hg
{
	PatternManager::PatternManager(HexagonGame* mHexagonGamePtr) :
		hgPtr{mHexagonGamePtr}, timeline(hgPtr->timeline), sides{hgPtr->sides}, centerPos(hgPtr->centerPos)
	{
	}

	void PatternManager::wall(Vector2f mCenterPos, int mSide, float mThickness, float mSpeed)
	{
		Entity* result { new Entity };
		hgPtr->manager.addEntity(result);
		result->addComponent(new CWall { hgPtr, mCenterPos, mSide, mThickness, 800, mSpeed * hgPtr->speedMult });
	}
	void PatternManager::mirrorWall(Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, int mSides = 1)
	{
		int mirrorSide { mSide + hgPtr->getSides() / 2 };

		for (int i { 0 }; i < mSides; i++)
		{
			wall(mCenterPos, mSide + i, mThickness, mSpeed);
			wall(mCenterPos, mirrorSide + i, mThickness, mSpeed);
		}
	}
	void PatternManager::barrage(Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, int mOpenSides = 1)
	{
		for(int i { 0 }; i < hgPtr->sides - mOpenSides; i++)
			wall(mCenterPos, mSide + i, mThickness, mSpeed);
	}
	void PatternManager::barrageDiv(Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, int mDiv)
	{
		for(int i { 0 }; i < hgPtr->sides / mDiv; i++)
			wall(mCenterPos, mSide + i * mDiv, mThickness, mSpeed);
	}

	void PatternManager::alternateBarrageDiv(int mDiv = 2)
	{
		float delay { 19 };
		float thickness { 25 };
		float speed = { 8 };

		for(int i { 0 }; i < 10; i++)
		{
			timeline.add(new Do{[=](){ barrageDiv(centerPos, i % mDiv, thickness, speed, mDiv); }});
			timeline.add(new Wait{delay});
		}
	}
	void PatternManager::spin(int mTimes)
	{
		float delay { 19 };
		float thickness { 25 };
		float speed = { 8 };

		for(int i = 0; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ barrage(centerPos, i, thickness, speed); }});
			timeline.add(new Wait{delay});
		}
	}
	void PatternManager::zigZag(int mTimes)
	{
		float delay { 39 };
		float thickness { 25 };
		float speed = { 8 };

		timeline.add(new Do{[=](){ barrage(centerPos, 0, thickness, speed); }});
		timeline.add(new Wait{delay});
		timeline.add(new Do{[=](){ barrage(centerPos, sides / 2, thickness, speed); }});
		timeline.add(new Wait{delay});
		timeline.add(new Goto{0, mTimes});
	}
	void PatternManager::mirrorSpin(int mTimes)
	{
		float delay { 19 };
		float thickness { 25 };
		float speed = { 8 };

		for(int i { 0 }; i < mTimes; i++)
		{
			timeline.add(new Do{[=](){ mirrorWall(centerPos, i, thickness, speed, 2); }});
			timeline.add(new Wait{delay});
		}
	}
}

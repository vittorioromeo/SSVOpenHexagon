// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

using namespace sf;
using namespace sses;

namespace hg
{
	Entity* createWall(Manager& mManager, HexagonGame* mHgPtr, Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, float mSpeedMultiplier)
	{
		auto result = mManager.createEntity();
		*result += mManager.createComponent<CWall>(*mHgPtr, mCenterPos, mSide, mThickness, getSpawnDistance(), mSpeed * mSpeedMultiplier);
		return result;
	}
	Entity* createPlayer(Manager& mManager, HexagonGame* mHgPtr, Vector2f mCenterPos)
	{
		auto result = mManager.createEntity();
		*result += mManager.createComponent<CPlayer>(*mHgPtr, mCenterPos);
		result->setDrawPriority(-1);
		return result;
	}
}

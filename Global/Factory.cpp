#include <SSVStart.h>
#include <SSVEntitySystem.h>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

namespace hg
{
	Entity* createWall(Manager& mManager, HexagonGame* mHgPtr, Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, float mSpeedMultiplier)
	{
		Entity* result { new Entity };
		mManager.addEntity(result);
		result->addComponent(new CWall { mHgPtr, mCenterPos, mSide, mThickness, getSpawnDistance(), mSpeed * mSpeedMultiplier });
		return result;
	}
	Entity* createPlayer(Manager& mManager, HexagonGame* mHgPtr, Vector2f mCenterPos)
	{
		Entity* result { new Entity };
		mManager.addEntity(result);
		result->addComponent(new CPlayer { mHgPtr, mCenterPos } );
		result->drawPriority = -1;
		return result;
	}
}

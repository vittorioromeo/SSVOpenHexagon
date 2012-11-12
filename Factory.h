#include "SSVStart.h"
#include "SSVEntitySystem.h"

namespace hg
{
	Entity* createWall(Manager& mManager, HexagonGame* mHgPtr, Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, float mSpeedMultiplier);
	Entity* createPlayer(Manager& mManager, HexagonGame* mHgPtr, Vector2f mCenterPos);
}

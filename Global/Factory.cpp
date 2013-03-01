// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Global/Factory.h"
#include "Utils/Utils.h"

using namespace sf;
using namespace sses;

namespace hg
{
	Factory::Factory(HexagonGame& mHexagonGame, Manager& mManager, Vector2f mCenterPos) : hexagonGame(mHexagonGame), manager(mManager), centerPos{mCenterPos} { }

	Entity& Factory::createWall(int mSide, float mThickness, float mSpeed, float mSpeedMultiplier)
	{
		auto& result = manager.createEntity();
		result += manager.createComponent<CWall>(hexagonGame, centerPos, mSide, mThickness, getSpawnDistance(), mSpeed * mSpeedMultiplier);
		return result;
	}
	Entity& Factory::createPlayer()
	{
		auto& result = manager.createEntity();
		result += manager.createComponent<CPlayer>(hexagonGame, centerPos);
		result.setDrawPriority(-1);
		return result;
	}
}

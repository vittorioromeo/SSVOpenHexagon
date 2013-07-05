// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CPlayer.h"
#include "SSVOpenHexagon/Components/CWall.h"
#include "SSVOpenHexagon/Global/Factory.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace sf;
using namespace sses;
using namespace ssvs;

namespace hg
{
	Factory::Factory(HexagonGame& mHexagonGame, Manager& mManager, Vec2f mCenterPos) : hexagonGame(mHexagonGame), manager(mManager), centerPos{mCenterPos} { }

	Entity& Factory::createWall(int mSide, float mThickness, float mSpeedMultiplier, float mAcceleration, float mMinSpeed, float mMaxSpeed)
	{
		auto& result(manager.createEntity("wall"));
		result.createComponent<CWall>(hexagonGame, centerPos, mSide, mThickness, getSpawnDistance(), mSpeedMultiplier, mAcceleration, mMinSpeed, mMaxSpeed);
		return result;
	}
	Entity& Factory::createWallHMod(float mHueModifier, int mSide, float mThickness, float mSpeedMultiplier, float mAcceleration, float mMinSpeed, float mMaxSpeed)
	{
		auto& result(Factory::createWall(mSide, mThickness, mSpeedMultiplier, mAcceleration, mMinSpeed, mMaxSpeed));
		auto& wall(result.getFirstComponent<CWall>("wall"));
		wall.setHueModifier(mHueModifier);
		return result;
	}
	Entity& Factory::createWallHModData(float mHueModifier, int mSide, float mThickness, SpeedData mSpeed, SpeedData mCurve)
	{
		auto& result(Factory::createWall(mSide, mThickness, mSpeed.speed, mSpeed.accel, mSpeed.min, mSpeed.max));
		auto& wall(result.getFirstComponent<CWall>("wall"));
		wall.setHueModifier(mHueModifier);
		wall.getSpeed() = mSpeed;
		wall.getCurve() = mCurve;
		return result;
	}
	Entity& Factory::createPlayer()
	{
		auto& result(manager.createEntity("player"));
		result.createComponent<CPlayer>(hexagonGame, centerPos);
		result.setDrawPriority(-1);
		return result;
	}
}

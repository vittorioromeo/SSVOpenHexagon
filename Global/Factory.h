// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef FACTORY_H_
#define FACTORY_H_

#include <SSVStart.h>
#include <SSVEntitySystem.h>

namespace hg
{
	sses::Entity* createWall(sses::Manager& mManager, HexagonGame* mHgPtr, sf::Vector2f mCenterPos, int mSide, float mThickness, float mSpeed, float mSpeedMultiplier);
	sses::Entity* createPlayer(sses::Manager& mManager, HexagonGame* mHgPtr, sf::Vector2f mCenterPos);
}

#endif /* FACTORY_H_ */

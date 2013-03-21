// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_FACTORY
#define HG_FACTORY

#include <SSVStart/SSVStart.h>
#include <SSVEntitySystem.h>

namespace hg
{
	class HexagonGame;

	class Factory
	{
		private:
			HexagonGame& hexagonGame;
			sses::Manager& manager;
			sf::Vector2f centerPos;

		public:
			Factory(HexagonGame& mHexagonGame, sses::Manager& mManager, sf::Vector2f mCenterPos);

			sses::Entity& createWall(int mSide, float mThickness, float mSpeedMultiplier, float mAcceleration = 0, float mMinSpeed = 0, float mMaxSpeed = 0);
			sses::Entity& createPlayer();
	};
}

#endif

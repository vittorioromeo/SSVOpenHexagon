// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef FACTORY_H_
#define FACTORY_H_

#include <SSVStart.h>
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

			sses::Entity& createWall(int mSide, float mThickness, float mSpeed, float mSpeedMultiplier);
			sses::Entity& createPlayer();
	};
}

#endif /* FACTORY_H_ */

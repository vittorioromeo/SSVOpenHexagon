// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_CPLAYER
#define HG_CPLAYER

#include "SSVOpenHexagon/Core/HGDependencies.h"
#include "SSVOpenHexagon/Global/Config.h"

namespace hg
{
	class HexagonGame;

	class CPlayer : public sses::Component
	{
		private:
			HexagonGame& hexagonGame;
			ssvs::Vec2f pLeft, pRight, startPos, pos;
			sf::VertexArray vertices{sf::PrimitiveType::Triangles, 3};
			float hue{0}, angle{0}, size{Config::getPlayerSize()}, speed{Config::getPlayerSpeed()}, focusSpeed{Config::getPlayerFocusSpeed()};
			bool dead{false};
			ssvs::Ticker swapTimer{36.f}, swapBlinkTimer{5.f}, deadEffectTimer{80.f, false};

			void drawPivot();
			void drawDeathEffect();

		public:
			CPlayer(HexagonGame& mHexagonGame, const ssvs::Vec2f& mStartPos);

			void update(float mFT) override;
			void draw() override;
	};
}

#endif

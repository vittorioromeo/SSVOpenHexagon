// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef CPLAYER_H_
#define CPLAYER_H_

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SSVEntitySystem.h>
#include "Global/Config.h"
#include "HexagonGame.h"

namespace hg
{
	class CPlayer : public sses::Component
	{
		private:
			HexagonGame& hexagonGame;
			sf::Vector2f pLeft, pRight, startPos, pos;
			sf::VertexArray vertices{sf::PrimitiveType::Triangles, 3};
			float hue{0}, angle{0}, size{getPlayerSize()}, speed{getPlayerSpeed()}, focusSpeed{getPlayerFocusSpeed()};
			bool dead{false};

			void drawPivot();			

		public:		
			CPlayer(HexagonGame& mHexagonGame, sf::Vector2f mStartPos);

			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CPLAYER_H_ */

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

using namespace sf;
using namespace sses;

namespace hg
{
	class CPlayer : public Component
	{
		private:
			HexagonGame* hgPtr;
			Vector2f pLeft, pRight, startPos, pos;
			VertexArray vertices{PrimitiveType::Triangles, 3};
			float hue{0};
			float size{getPlayerSize()};
			float angle{0};
			float speed{getPlayerSpeed()};
			float focusSpeed{getPlayerFocusSpeed()};
			bool isDead{false};

			inline void drawPivot();			

		public:		
			CPlayer(HexagonGame*, Vector2f);

			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CPLAYER_H_ */

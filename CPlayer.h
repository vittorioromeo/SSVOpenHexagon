#ifndef CPLAYER_H_
#define CPLAYER_H_

#include "SSVEntitySystem.h"
#include "HexagonGame.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "Config.h"

using namespace sf;
using namespace sses;

namespace hg
{
	class CPlayer: public Component
	{
		private:
			HexagonGame* hexagonGamePtr;
			Vector2f pTop, pLeft, pRight;
			Vector2f startPos, pos;

			VertexArray vertices { PrimitiveType::Triangles, 3 };

			inline void drawPivot();

		public:
			float size { getPlayerSize() };
			float angle { 0 };
			float speed { getPlayerSpeed() };
			float focusSpeed { getPlayerFocusSpeed() };

			CPlayer(HexagonGame*, Vector2f);

			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CPLAYER_H_ */

#ifndef CPLAYER_H_
#define CPLAYER_H_

#include "SSVEntitySystem.h"
#include "HexagonGame.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

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

			inline void drawPivot();

		public:
			float size { 7 };
			float angle { 0 };
			float speed { 6.6f };
			float focusSpeed { 3.3f };

			CPlayer(HexagonGame*, Vector2f);

			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CPLAYER_H_ */

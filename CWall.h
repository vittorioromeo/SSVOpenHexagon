#ifndef CWALL_H_
#define CWALL_H_

#include "SSVEntitySystem.h"
#include "HexagonGame.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace sses;

namespace hg
{
	class CWall : public Component
	{
		private:
			HexagonGame* hexagonGamePtr;
			Vector2f centerPos, p1, p2, p3, p4;
			float speed;
			VertexArray vertices { PrimitiveType::Quads, 4 };

		public:
			vector<Vector2f*> pointPtrs { &p1, &p2, &p3, &p4 };

			CWall(HexagonGame*, Vector2f, int, float, float, float);

			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CWALL_H_ */

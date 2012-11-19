#ifndef CWALL_H_
#define CWALL_H_

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SSVEntitySystem.h>
#include "HexagonGame.h"

using namespace sf;
using namespace sses;

namespace hg
{
	class CWall : public Component
	{
		private:
			HexagonGame* hgPtr;
			Vector2f centerPos, p1, p2, p3, p4;			
			VertexArray vertices{PrimitiveType::Quads, 4};
			vector<Vector2f*> pointPtrs{&p1, &p2, &p3, &p4};
			float speed{0};

		public:
			CWall(HexagonGame*, Vector2f, int, float, float, float);

			bool isOverlapping(Vector2f mPoint);
			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CWALL_H_ */

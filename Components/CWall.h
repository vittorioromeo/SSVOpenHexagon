// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

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
			Vector2f centerPos;
			vector<Vector2f> vertexPositions{4};
			VertexArray vertices{PrimitiveType::Quads, 4};
			vector<Vector2f*> pointPtrs{&vertexPositions[0], &vertexPositions[1], &vertexPositions[2], &vertexPositions[3]};
			float speed{0};
			float distance{0};
			float thickness{0};
			int side{0};

		public:
			CWall(HexagonGame*, Vector2f, int, float, float, float);

			bool isOverlapping(Vector2f mPoint);
			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CWALL_H_ */

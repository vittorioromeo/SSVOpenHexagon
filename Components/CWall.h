// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_CWALL
#define HG_CWALL

#include <SFML/Graphics.hpp>
#include <SSVEntitySystem.h>

namespace hg
{
	class HexagonGame;

	class CWall : public sses::Component
	{
		private:
			HexagonGame& hexagonGame;
			sf::Vector2f centerPos;
			std::vector<sf::Vector2f> vertexPositions{4};
			sf::VertexArray vertices{sf::PrimitiveType::Quads, 4};
			std::vector<sf::Vector2f*> pointPtrs{&vertexPositions[0], &vertexPositions[1], &vertexPositions[2], &vertexPositions[3]};
			float speed{0}, distance{0}, thickness{0};
			int side{0};

		public:
			CWall(HexagonGame& mHexagonGame, sf::Vector2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed);

			bool isOverlapping(sf::Vector2f mPoint);
			void update(float mFrameTime) override;
			void draw() override;
	};
}

#endif

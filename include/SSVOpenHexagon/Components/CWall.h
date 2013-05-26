// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_CWALL
#define HG_CWALL

#include <SFML/Graphics.hpp>
#include <SSVEntitySystem/SSVEntitySystem.h>

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
			float speed{0}, distance{0}, thickness{0}, acceleration{0}, minSpeed{0}, maxSpeed{0};
			int side{0};

		public:
			CWall(sses::Entity& mEntity, HexagonGame& mHexagonGame, sf::Vector2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed,
				float mAcceleration = 0, float mMinSpeed = 0, float mMaxSpeed = 0);

			bool isOverlapping(sf::Vector2f mPoint);
			void update(float mFrameTime) override;
			void draw() override;
	};
}

#endif

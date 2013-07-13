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

	struct SpeedData
	{
		float speed{0}, accel{0}, min{0}, max{0};
		bool pingPong{false};

		SpeedData() = default;
		SpeedData(float mSpeed, float mAccel, float mMin, float mMax, bool mPingPong)
			: speed{mSpeed}, accel{mAccel}, min{mMin}, max{mMax}, pingPong{mPingPong} { }

		void update(float mFrameTime)
		{
			if(accel == 0) return;
			speed += accel * mFrameTime;
			if(speed > max)
			{
				speed = max;
				if(pingPong) accel *= -1;
			}
			if(speed < min)
			{
				speed = min;
				if(pingPong) accel *= -1;
			}
		}
	};

	class CWall : public sses::Component
	{
		private:
			HexagonGame& hexagonGame;
			ssvs::Vec2f centerPos;
			std::vector<ssvs::Vec2f> vertexPositions{4};
			sf::VertexArray vertices{sf::PrimitiveType::Quads, 4};
			SpeedData speed, curve;
			float distance{0}, thickness{0}, hueModifier{0};
			int side{0};

		public:
			CWall(HexagonGame& mHexagonGame, ssvs::Vec2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed, float mAcceleration = 0, float mMinSpeed = 0, float mMaxSpeed = 0);

			bool isOverlapping(ssvs::Vec2f mPoint) const;
			void update(float mFrameTime) override;
			void draw() override;

			void setHueModifier(float mHueModifier);
			SpeedData& getSpeed();
			SpeedData& getCurve();
	};
}

#endif

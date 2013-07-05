// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.h"
#include "SSVOpenHexagon/Components/CPlayer.h"
#include "SSVOpenHexagon/Components/CWall.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace sf;
using namespace sses;
using namespace ssvs;
using namespace ssvs::Utils;

namespace hg
{
	CWall::CWall(Entity& mEntity, HexagonGame& mHexagonGame, Vec2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed,
		float mAcceleration, float mMinSpeed, float mMaxSpeed) : Component{mEntity, "wall"}, hexagonGame(mHexagonGame), centerPos{mCenterPos},
		speed{mSpeed}, distance{mDistance}, thickness{mThickness}, acceleration{mAcceleration}, minSpeed{mMinSpeed}, maxSpeed{mMaxSpeed}, side{mSide}
	{
		float div{360.f / hexagonGame.getSides()}, angle{div * side};

		vertexPositions[0] = getOrbitFromDegrees(centerPos, angle - div * 0.5f, distance);
		vertexPositions[1] = getOrbitFromDegrees(centerPos, angle + div * 0.5f, distance);
		vertexPositions[2] = getOrbitFromDegrees(centerPos, angle + div * 0.5f + hexagonGame.getWallAngleLeft(), distance + thickness + hexagonGame.getWallSkewLeft());
		vertexPositions[3] = getOrbitFromDegrees(centerPos, angle - div * 0.5f + hexagonGame.getWallAngleRight(), distance + thickness + hexagonGame.getWallSkewRight());
	}

	bool CWall::isOverlapping(Vec2f mPoint) const { return isPointInPolygon(vertexPositions, mPoint); }

	void CWall::draw()
	{
		const auto& colorMain(hexagonGame.getColorMain());

		for(unsigned int i{0}; i < 4; ++i)
		{
			vertices[i].position = vertexPositions[i];
			vertices[i].color = colorMain;
		}

		hexagonGame.render(vertices);
	}

	void CWall::update(float mFrameTime)
	{
		if(acceleration != 0)
		{
			speed += acceleration * mFrameTime;
			if(speed > maxSpeed) speed = maxSpeed;
			if(speed < minSpeed) speed = minSpeed;
		}

		float radius{hexagonGame.getRadius() * 0.65f};
		int pointsOnCenter{0};

		for(auto& vp : vertexPositions)
		{
			if(abs(vp.x - centerPos.x) < radius && abs(vp.y - centerPos.y) < radius) pointsOnCenter++;
			else vp = getMovedTowards(vp, centerPos, speed * 5.0f * mFrameTime);
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


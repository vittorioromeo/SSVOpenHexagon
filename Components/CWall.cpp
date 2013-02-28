// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

using namespace sf;
using namespace sses;
using namespace ssvs::Utils;

namespace hg
{
	CWall::CWall(HexagonGame& mHexagonGame, Vector2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed) :
			Component{"wall"}, hexagonGame(mHexagonGame), centerPos{mCenterPos}, speed{mSpeed}, distance{mDistance}, thickness{mThickness}, side{mSide}
	{
		float div{360.f / hexagonGame.getSides()};
		float angle{div * side};

		vertexPositions[0] = getOrbit(centerPos, angle - div * 0.5f, distance);
		vertexPositions[1] = getOrbit(centerPos, angle + div * 0.5f, distance);
		vertexPositions[2] = getOrbit(centerPos, angle + div * 0.5f + hexagonGame.getWallAngleLeft(), distance + thickness + hexagonGame.getWallSkewLeft());
		vertexPositions[3] = getOrbit(centerPos, angle - div * 0.5f + hexagonGame.getWallAngleRight(), distance + thickness + hexagonGame.getWallSkewRight());
	}

	bool CWall::isOverlapping(Vector2f mPoint) { return isPointInPolygon(pointPtrs, mPoint); }

	void CWall::draw()
	{
		for(int i{0}; i < 4; i++)
		{
			vertices[i].position = vertexPositions[i];
			vertices[i].color = hexagonGame.getColorMain();
		}
		
		hexagonGame.render(vertices);
	}

	void CWall::update(float mFrameTime)
	{
		float radius{hexagonGame.getRadius() * 0.65f};
		int pointsOnCenter{0};

		for(auto& vertexPosition : vertexPositions)
		{
			int distanceX{abs(vertexPosition.x - centerPos.x)};
			int distanceY{abs(vertexPosition.y - centerPos.y)};

			if(distanceX < radius && distanceY < radius) pointsOnCenter++;
			else movePointTowardsCenter(vertexPosition, centerPos, speed * mFrameTime);
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


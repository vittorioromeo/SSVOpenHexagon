// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include "Components/CPlayer.h"
#include "Components/CWall.h"
#include "Utils/Utils.h"

using namespace ssvs::Utils;

namespace hg
{
	CWall::CWall(HexagonGame *mHgPtr, Vector2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed) :
			Component{"wall"}, hgPtr{mHgPtr}, centerPos{mCenterPos}, speed{mSpeed}, distance{mDistance}, thickness{mThickness}, side{mSide}
	{
		float div{360.f / hgPtr->getSides()};
		float angle{div * side};

		vertexPositions[0] = getOrbit(centerPos, angle - div * 0.5f, distance);
		vertexPositions[1] = getOrbit(centerPos, angle + div * 0.5f, distance);
		vertexPositions[2] = getOrbit(centerPos, angle + div * 0.5f + hgPtr->getWallAngleLeft(), distance + thickness + hgPtr->getWallSkewLeft());
		vertexPositions[3] = getOrbit(centerPos, angle - div * 0.5f + hgPtr->getWallAngleRight(), distance + thickness + hgPtr->getWallSkewRight());
	}

	bool CWall::isOverlapping(Vector2f mPoint) { return isPointInPolygon(pointPtrs, mPoint); }

	void CWall::draw()
	{
		for (int i{0}; i < 4; i++)
		{
			vertices[i].position = vertexPositions[i];
			vertices[i].color = hgPtr->getColorMain();
		}
		
		hgPtr->drawOnTexture(vertices);
	}

	void CWall::update(float mFrameTime)
	{
		float radius{hgPtr->getRadius() * 0.65f};
		int pointsOnCenter{0};

		for(auto& vertexPosition : vertexPositions)
		{
			float distanceX{abs(vertexPosition.x - centerPos.x)};
			float distanceY{abs(vertexPosition.y - centerPos.y)};

			if(distanceX < radius && distanceY < radius) pointsOnCenter++;
			else movePointTowardsCenter(vertexPosition, centerPos, speed * mFrameTime);
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


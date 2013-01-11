/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

		p1 = getOrbit(centerPos, angle - div * 0.5f, distance);
		p2 = getOrbit(centerPos, angle + div * 0.5f, distance);
		p3 = getOrbit(centerPos, angle + div * 0.5f + hgPtr->getWallAngleLeft(), distance + thickness + hgPtr->getWallSkewLeft());
		p4 = getOrbit(centerPos, angle - div * 0.5f + hgPtr->getWallAngleRight(), distance + thickness + hgPtr->getWallSkewRight());
	}

	bool CWall::isOverlapping(Vector2f mPoint) { return isPointInPolygon(pointPtrs, mPoint); }

	void CWall::draw()
	{
		Color color{hgPtr->getColorMain()};

		vertices[0].position = p1;
		vertices[1].position = p2;
		vertices[2].position = p3;
		vertices[3].position = p4;

		vertices[0].color = color;
		vertices[1].color = color;
		vertices[2].color = color;
		vertices[3].color = color;

		hgPtr->drawOnTexture(vertices);
	}

	void CWall::update(float mFrameTime)
	{
		float radius{hgPtr->getRadius() * 0.65f};
		int pointsOnCenter{0};

		for(auto pointPtr : pointPtrs)
		{
			float distanceX{abs(pointPtr->x - centerPos.x)};
			float distanceY{abs(pointPtr->y - centerPos.y)};

			if(distanceX < radius && distanceY < radius) pointsOnCenter++;
			else movePointTowardsCenter(*pointPtr, centerPos, speed * mFrameTime);
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


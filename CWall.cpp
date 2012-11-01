#include "CWall.h"
#include "Utils.h"
#include "CPlayer.h"
#include <iostream>

namespace hg
{
	void movePoint(Vector2f &mVector, const Vector2f mCenter, const float mSpeed)
	{
		Vector2f m = mCenter - mVector;
		m = normalize(m);
		m *= mSpeed;
		mVector += m;
	}

	CWall::CWall(HexagonGame *mHexagonGamePtr, Vector2f mCenterPos, int mSide, float mThickness, float mDistance, float mSpeed) :
			Component("wall"), hexagonGamePtr(mHexagonGamePtr), centerPos(mCenterPos), speed(mSpeed)
	{
		float div = 360.f / hexagonGamePtr->getSides();
		float angle = div * mSide ;

		p1 = orbit(centerPos, angle - div * 0.5f, mDistance);
		p2 = orbit(centerPos, angle + div * 0.5f, mDistance);
		p3 = orbit(centerPos, angle + div * 0.5f, mDistance + mThickness);
		p4 = orbit(centerPos, angle - div * 0.5f, mDistance + mThickness);
	}

	void CWall::draw()
	{
		Color color = hexagonGamePtr->getColor();
		VertexArray vertices(PrimitiveType::Quads, 4);
		vertices.append(Vertex(p1, color));
		vertices.append(Vertex(p2, color));
		vertices.append(Vertex(p3, color));
		vertices.append(Vertex(p4, color));
		hexagonGamePtr->drawOnTexture(vertices);
	}

	void CWall::update(float mFrameTime)
	{
		float radius = hexagonGamePtr->getRadius() * 0.65f;
		int pointsOnCenter(0);

		for(auto pointPtr : pointPtrs)
		{
			int distanceX = abs(pointPtr->x - centerPos.x);
			int distanceY = abs(pointPtr->y - centerPos.y);

			if(distanceX < radius && distanceY < radius) pointsOnCenter++;
			else movePoint(*pointPtr, centerPos, speed * mFrameTime);
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


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

namespace hg
{
	CWall::CWall(HexagonGame& mHexagonGame, const Vec2f& mCenterPos, int mSide, float mThickness, float mDistance, const SpeedData& mSpeed, const SpeedData& mCurve) : hexagonGame(mHexagonGame), centerPos{mCenterPos},
		speed{mSpeed}, curve{mCurve}, distance{mDistance}, thickness{mThickness}, side{mSide}
	{
		float div{360.f / hexagonGame.getSides() * 0.5f}, angle{div * 2 * side};

		vertexPositions[0] = getOrbitFromDegrees(centerPos, angle - div, distance);
		vertexPositions[1] = getOrbitFromDegrees(centerPos, angle + div, distance);
		vertexPositions[2] = getOrbitFromDegrees(centerPos, angle + div + hexagonGame.getWallAngleLeft(), distance + thickness + hexagonGame.getWallSkewLeft());
		vertexPositions[3] = getOrbitFromDegrees(centerPos, angle - div + hexagonGame.getWallAngleRight(), distance + thickness + hexagonGame.getWallSkewRight());
	}

	void CWall::draw()
	{
		auto colorMain(hexagonGame.getColorMain());
		if(hueMod != 0 && !hexagonGame.getStatus().drawing3D) colorMain = Utils::transformHue(colorMain, hueMod);

		for(auto i(0u); i < 4; ++i)
		{
			auto& vertex(vertices[i]);
			vertex.position = vertexPositions[i];
			vertex.color = colorMain;
		}

		hexagonGame.render(vertices);
	}

	void CWall::update(float mFrameTime)
	{
		speed.update(mFrameTime);
		curve.update(mFrameTime);

		float radius{hexagonGame.getRadius() * 0.65f};
		int pointsOnCenter{0};

		for(auto& vp : vertexPositions)
		{
			if(abs(vp.x - centerPos.x) < radius && abs(vp.y - centerPos.y) < radius) pointsOnCenter++;
			else
			{
				moveTowards(vp, centerPos, speed.speed * 5.0f * mFrameTime);
				rotateAroundCenter(vp, centerPos, curve.speed / 60.f * mFrameTime);
			}
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


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

		vertexPositions[0] = getOrbitFromDeg(centerPos, angle - div, distance);
		vertexPositions[1] = getOrbitFromDeg(centerPos, angle + div, distance);
		vertexPositions[2] = getOrbitFromDeg(centerPos, angle + div + hexagonGame.getWallAngleLeft(), distance + thickness + hexagonGame.getWallSkewLeft());
		vertexPositions[3] = getOrbitFromDeg(centerPos, angle - div + hexagonGame.getWallAngleRight(), distance + thickness + hexagonGame.getWallSkewRight());
	}

	void CWall::draw()
	{
		auto colorMain(hexagonGame.getColorMain());
		if(hueMod != 0 && !hexagonGame.getStatus().drawing3D) colorMain = Utils::transformHue(colorMain, hueMod);

		for(auto i(0u); i < 4; ++i) hexagonGame.wallQuads.emplace_back(vertexPositions[i], colorMain);
	}

	void CWall::update(float mFT)
	{
		speed.update(mFT);
		curve.update(mFT);

		float radius{hexagonGame.getRadius() * 0.65f};
		int pointsOnCenter{0};

		for(auto& vp : vertexPositions)
		{
			if(abs(vp.x - centerPos.x) < radius && abs(vp.y - centerPos.y) < radius) pointsOnCenter++;
			else
			{
				moveTowards(vp, centerPos, speed.speed * 5.f * mFT);
				rotateRadAroundCenter(vp, centerPos, curve.speed / 60.f * mFT);
			}
		}

		if(pointsOnCenter > 3) getEntity().destroy();
	}
}


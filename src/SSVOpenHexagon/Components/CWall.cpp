// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

using namespace sf;
using namespace ssvs;

namespace hg
{

CWall::CWall(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos, int mSide,
    float mThickness, float mDistance, const SpeedData& mSpeed,
    const SpeedData& mCurve)
    : speed{mSpeed}, curve{mCurve}
{
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float angle{div * 2.f * mSide};

    vertexPositions[0] = getOrbitRad(mCenterPos, angle - div, mDistance);
    vertexPositions[1] = getOrbitRad(mCenterPos, angle + div, mDistance);
    vertexPositions[2] =
        getOrbitRad(mCenterPos, angle + div + mHexagonGame.getWallAngleLeft(),
            mDistance + mThickness + mHexagonGame.getWallSkewLeft());
    vertexPositions[3] =
        getOrbitRad(mCenterPos, angle - div + mHexagonGame.getWallAngleRight(),
            mDistance + mThickness + mHexagonGame.getWallSkewRight());
}

void CWall::draw(HexagonGame& mHexagonGame)
{
    Color colorMain(mHexagonGame.getColorMain());

    if(hueMod != 0)
    {
        colorMain = Utils::transformHue(colorMain, hueMod);
    }

    mHexagonGame.wallQuads.reserve_more(4);
    mHexagonGame.wallQuads.batch_unsafe_emplace_back(colorMain,
        vertexPositions[0], vertexPositions[1], vertexPositions[2],
        vertexPositions[3]);
}

void CWall::update(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos, FT mFT)
{
    speed.update(mFT);
    curve.update(mFT);

    const float radius{mHexagonGame.getRadius() * 0.65f};
    int pointsOnCenter{0};

    for(sf::Vector2f& vp : vertexPositions)
    {
        if(std::abs(vp.x - mCenterPos.x) < radius &&
            std::abs(vp.y - mCenterPos.y) < radius)
        {
            ++pointsOnCenter;
        }
        else
        {
            moveTowards(vp, mCenterPos, speed.speed * 5.f * mFT);
            rotateRadAround(vp, mCenterPos, curve.speed / 60.f * mFT);
        }
    }

    if(pointsOnCenter > 3)
    {
        killed = true;
    }
}

} // namespace hg

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

namespace hg
{

CWall::CWall(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos,
    const int mSide, const float mThickness, const float mDistance,
    const SpeedData& mSpeed, const SpeedData& mCurve)
    : speed{mSpeed}, curve{mCurve}, hueMod{0}, killed{false}
{
    const float div{ssvu::tau / mHexagonGame.getSides() * 0.5f};
    const float angle{div * 2.f * mSide};

    vertexPositions[0] = ssvs::getOrbitRad(mCenterPos, angle - div, mDistance);
    vertexPositions[1] = ssvs::getOrbitRad(mCenterPos, angle + div, mDistance);
    vertexPositions[2] = ssvs::getOrbitRad(mCenterPos,
        angle + div + mHexagonGame.getWallAngleLeft(),
        mDistance + mThickness + mHexagonGame.getWallSkewLeft());
    vertexPositions[3] = ssvs::getOrbitRad(mCenterPos,
        angle - div + mHexagonGame.getWallAngleRight(),
        mDistance + mThickness + mHexagonGame.getWallSkewRight());
}

void CWall::draw(HexagonGame& mHexagonGame)
{
    sf::Color colorMain(mHexagonGame.getColorMain());

    if(hueMod != 0)
    {
        colorMain = Utils::transformHue(colorMain, hueMod);
    }

    mHexagonGame.wallQuads.reserve_more(4);
    mHexagonGame.wallQuads.batch_unsafe_emplace_back(colorMain,
        vertexPositions[0], vertexPositions[1], vertexPositions[2],
        vertexPositions[3]);
}

void CWall::update(HexagonGame& mHexagonGame,
    const sf::Vector2f& mCenterPos, const ssvu::FT mFT)
{
    speed.update(mFT);
    curve.update(mFT);

    moveTowardsCenter(mHexagonGame, mCenterPos, mFT);
    if(curve.speed != 0.f)
    {
        moveCurve(mCenterPos, mFT);
    }

    outOfPlayerRadius = false;
}

void CWall::moveTowardsCenter(HexagonGame& mHexagonGame,
    const sf::Vector2f& mCenterPos, const ssvu::FT mFT)
{
    const float wallSpawnDist{mHexagonGame.getLevelStatus().wallSpawnDistance};
    const float radius{mHexagonGame.getRadius() * 0.5f};
    const float outerBounds{wallSpawnDist * 1.1f};

    int pointsOutOfBounds{0}, pointsOnCenter{0};
    float xDistance, yDistance;

    for(sf::Vector2f& vp : vertexPositions)
    {
        xDistance = std::abs(vp.x - mCenterPos.x);
        yDistance = std::abs(vp.y - mCenterPos.y);

        if(xDistance < radius && yDistance < radius)
        {
            ++pointsOnCenter;
            continue;
        }
        if(xDistance > outerBounds || yDistance > outerBounds)
        {
            ++pointsOutOfBounds;
        }

        ssvs::moveTowards(vp, mCenterPos, speed.speed * 5.f * mFT);
    }

    if(pointsOnCenter > 3 || pointsOutOfBounds > 3)
    {
        killed = true;
    }
}

void CWall::moveCurve(const sf::Vector2f& mCenterPos,
    const ssvu::FT mFT)
{
    for(sf::Vector2f& vp : vertexPositions)
    {
        moveVertexAlongCurve(vp, mCenterPos, mFT);
    }
}

void CWall::setHueMod(float mHueMod) noexcept
{
    hueMod = mHueMod;
}

} // namespace hg

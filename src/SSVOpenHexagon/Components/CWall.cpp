// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"

using namespace hg::Utils;

namespace hg
{

CWall::CWall(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos,
    int mSide, float mThickness, float mDistance, const SpeedData& mSpeed,
    const SpeedData& mCurve)
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

inline constexpr unsigned int wallSides{4};

void CWall::draw(HexagonGame& mHexagonGame)
{
    sf::Color colorMain(mHexagonGame.getColorMain());

    if(hueMod != 0)
    {
        colorMain = Utils::transformHue(colorMain, hueMod);
    }

    mHexagonGame.wallQuads.reserve_more(wallSides);
    mHexagonGame.wallQuads.batch_unsafe_emplace_back(colorMain,
        vertexPositions[0], vertexPositions[1], vertexPositions[2],
        vertexPositions[3]);
}

unsigned int CWall::getVertexScreenPortion(const sf::Vector2f& mVertex,
    const sf::Vector2f& mCenterPos, const unsigned int mSides)
{
    const float div{ssvu::tau / mSides},
        vertexAngle{ssvu::getWrapRad(ssvs::getRad(mVertex, mCenterPos))};
    float angle{div};
    unsigned int i{1};

    if(mSides % 2 == 0)
    {
        angle = div / 2.f;
        i = 0;
    }

    for(; i < mSides + i; ++i)
    {
        if(vertexAngle < angle)
        {
            break;
        }
        angle += div;
    }

    return (mSides / 2 + i) % mSides;
}

void CWall::calcIntersectionPoint(const HexagonGame& mHexagonGame,
    sf::Vector2f& mIntersection,
    const std::vector<sf::Vector2f>& mPivotVertexes,
    const sf::Vector2f& mCenterPos, const sf::Vector2f& wallVertexOne,
    const sf::Vector2f& wallVertexTwo)
{
    // Find which side of the polygon is intersected by the wall side.
    const unsigned int sides{mHexagonGame.getSides()};
    const unsigned int side{
        getVertexScreenPortion(wallVertexOne, mCenterPos, sides)};

    if(!getLinesIntersection(mIntersection, wallVertexOne, wallVertexTwo,
           mPivotVertexes[side], mPivotVertexes[(side + 1) % sides]))
    {
        // this function should never return false in this context.
        SSVU_ASSERT(false);
    }

    // If the result intersection does not fall within the boundaries of the
    // wall segment set it to be the original extremity of the wall.
    if(!(std::abs(wallVertexOne.x - mIntersection.x) <
               std::abs(wallVertexOne.x - wallVertexTwo.x) &&
           std::abs(wallVertexOne.y - mIntersection.y) <
               std::abs(wallVertexOne.y - wallVertexTwo.y)))
    {
        mIntersection = wallVertexOne;
    }
}

void CWall::draw3DSide(HexagonGame& mHexagonGame, const sf::Vector2f& mOffset3D,
    const sf::Color& mColor, const sf::Vector2f& mVertexOne,
    const sf::Vector2f& mVertexTwo)
{
    mHexagonGame.wallQuads3D.batch_unsafe_emplace_back(mColor, mVertexOne,
        mVertexTwo,
        sf::Vector2f{mVertexTwo.x + mOffset3D.x, mVertexTwo.y + mOffset3D.y},
        sf::Vector2f{mVertexOne.x + mOffset3D.x, mVertexOne.y + mOffset3D.y});
}

void CWall::draw3DSides(HexagonGame& mHexagonGame,
    const sf::Vector2f& mOffset3D, const sf::Color& mColor)
{
    mHexagonGame.wallQuads3D.reserve_more(4 * wallSides);

    for(unsigned int i{0}, j{wallSides - 1}; i < wallSides; j = i++)
    {
        mHexagonGame.wallQuads3D.batch_unsafe_emplace_back(mColor,
            vertexPositions[j], vertexPositions[i],
            sf::Vector2f{vertexPositions[i].x + mOffset3D.x,
                vertexPositions[i].y + mOffset3D.y},
            sf::Vector2f{vertexPositions[j].x + mOffset3D.x,
                vertexPositions[j].y + mOffset3D.y});
    }
}

void CWall::draw3D(HexagonGame& mHexagonGame, const CPlayer& mPlayer,
    const sf::Vector2f& mCenterPos, const sf::Color& mColor)
{
    // If all the vertexes are inside the pivot there is no point in putting
    // any effort in the drawing process.
    const float radius{mPlayer.getPivotRadius()};
    if(fastSqrt(ssvs::getMagSquared(vertexPositions[2] - mCenterPos)) <= radius)
    {
        return;
    }

    const sf::Vector2f& offset3D{mHexagonGame.get3DOffset()};

    //--------------------------------------------------------------------
    // Wall too far to cause overlapping issues.

    if(fastSqrt(ssvs::getMagSquared(vertexPositions[0] - mCenterPos)) > radius)
    {
        draw(mHexagonGame);
        draw3DSides(mHexagonGame, offset3D, mColor);
        return;
    }

    //--------------------------------------------------------------------
    // Wall close enough to the pivot to cause overlapping issues

    const std::vector<sf::Vector2f>& pivotVertexes{mPlayer.getPivotVertexes()};

    sf::Vector2f intersectionZero;
    calcIntersectionPoint(mHexagonGame, intersectionZero, pivotVertexes,
        mCenterPos, vertexPositions[0], vertexPositions[3]);

    sf::Vector2f intersectionOne;
    calcIntersectionPoint(mHexagonGame, intersectionOne, pivotVertexes,
        mCenterPos, vertexPositions[1], vertexPositions[2]);

    mHexagonGame.wallQuads3D.reserve_more(3 * wallSides);
    draw3DSide(
        mHexagonGame, offset3D, mColor, intersectionOne, vertexPositions[2]);
    draw3DSide(
        mHexagonGame, offset3D, mColor, vertexPositions[2], vertexPositions[3]);
    draw3DSide(
        mHexagonGame, offset3D, mColor, vertexPositions[3], intersectionZero);

    //--------------------------------------------------------------------
    // Standard top drawing

    sf::Color colorMain(mHexagonGame.getColorMain());

    if(hueMod != 0)
    {
        colorMain = Utils::transformHue(colorMain, hueMod);
    }

    mHexagonGame.wallQuads.reserve_more(wallSides);
    mHexagonGame.wallQuads.batch_unsafe_emplace_back(colorMain,
        intersectionZero, intersectionOne, vertexPositions[2],
        vertexPositions[3]);
}

void CWall::update(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos,
    const ssvu::FT mFT)
{
    speed.update(mFT);
    curve.update(mFT);

    moveTowardsCenter(mHexagonGame, mCenterPos, mFT);
    if(curve.speed != 0.f)
    {
        moveCurve(mCenterPos, mFT);
    }
}

void CWall::moveTowardsCenter(HexagonGame& mHexagonGame,
    const sf::Vector2f& mCenterPos, const ssvu::FT mFT)
{
    const float wallSpawnDist{mHexagonGame.getLevelStatus().wallSpawnDistance};
    const float radius{mHexagonGame.getRadius() * 0.5f};
    const float outerBounds{wallSpawnDist * 1.1f};

    int pointsOutOfBounds{0};
    int pointsOnCenter{0};

    for(sf::Vector2f& vp : vertexPositions)
    {
        const float xDistance = std::abs(vp.x - mCenterPos.x);
        const float yDistance = std::abs(vp.y - mCenterPos.y);

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

void CWall::moveCurve(const sf::Vector2f& mCenterPos, const ssvu::FT mFT)
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

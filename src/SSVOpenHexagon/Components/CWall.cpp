// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

namespace hg
{

CWall::CWall(HexagonGame& mHexagonGame, unsigned int mSide, float mThickness,
    float mDistance, const SpeedData& mSpeed, const SpeedData& mCurve)
    : initialSides{mHexagonGame.getSides()},
      side{(mHexagonGame.getSides() + (mSide % mHexagonGame.getSides())) %
           mHexagonGame.getSides()},
      distance{mDistance}, thickness{mThickness}, speed{mSpeed}, curve{mCurve}
{
}

void CWall::draw(HexagonGame& mHexagonGame)
{
    sf::Color colorMain(mHexagonGame.getColorMain());

    if(hueMod != 0)
    {
        colorMain = Utils::transformHue(colorMain, hueMod);
    }

    const auto fieldPos{mHexagonGame.getFieldPos()};
    const auto status{mHexagonGame.getStatus()};
    const auto styleData{mHexagonGame.getStyleData()};
    const auto levelStatus{mHexagonGame.getLevelStatus()};
    const auto fieldAngle =
        ssvu::toRad(styleData.bgRotOff + levelStatus.rotation);
    const float div{ssvu::tau / initialSides * 0.5f};
    const float col_angle{curveOffset + div * 2.f * side};
    const float angle{curveOffset + fieldAngle + div * 2.f * side};

    const sf::Vector2f skewEffect{
        styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D,
        styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};
    const sf::Vector2f skew{1.f, 1.f + skewEffect.y};


    const float radius{abs(mHexagonGame.getRadius() * 0.75f)};
    const auto _distance{ssvu::getClampedMin(distance, radius)};
    const auto _distanceThiccL{ssvu::getClampedMin(
        distance + thickness + mHexagonGame.getWallSkewLeft(), radius)};
    const auto _distanceThiccR{ssvu::getClampedMin(
        distance + thickness + mHexagonGame.getWallSkewRight(), radius)};

    // For calculating collisions and whatever
    const sf::Color colorDebug(255, 0, 0, 150);
    Collisions_vertexPositions[0] =
        ssvs::getOrbitRad(fieldPos, col_angle - div, _distance);
    Collisions_vertexPositions[1] =
        ssvs::getOrbitRad(fieldPos, col_angle + div, _distance);
    Collisions_vertexPositions[2] = ssvs::getOrbitRad(fieldPos,
        col_angle + div + mHexagonGame.getWallAngleLeft(), _distanceThiccL);
    Collisions_vertexPositions[3] = ssvs::getOrbitRad(fieldPos,
        col_angle - div + mHexagonGame.getWallAngleRight(), _distanceThiccR);

    // For drawing
    vertexPositions[0] = Utils::getSkewedOrbitRad(
        fieldPos, angle - div, _distance, styleData.skew);
    vertexPositions[1] = Utils::getSkewedOrbitRad(
        fieldPos, angle + div, _distance, styleData.skew);
    vertexPositions[2] = Utils::getSkewedOrbitRad(fieldPos,
        angle + div + mHexagonGame.getWallAngleLeft(), _distanceThiccL,
        styleData.skew);
    vertexPositions[3] = Utils::getSkewedOrbitRad(fieldPos,
        angle - div + mHexagonGame.getWallAngleRight(), _distanceThiccR,
        styleData.skew);

    mHexagonGame.wallDebugQuads.reserve_more(4);
    mHexagonGame.wallDebugQuads.batch_unsafe_emplace_back(colorDebug,
        Collisions_vertexPositions[0], Collisions_vertexPositions[1],
        Collisions_vertexPositions[2], Collisions_vertexPositions[3]);
    mHexagonGame.wallQuads.reserve_more(4);
    mHexagonGame.wallQuads.batch_unsafe_emplace_back(colorMain,
        vertexPositions[0], vertexPositions[1], vertexPositions[2],
        vertexPositions[3]);
}


void CWall::update(
    HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos, ssvu::FT mFT)
{
    speed.update(mFT);
    curve.update(mFT);

    const float radius{abs(mHexagonGame.getRadius() * 0.75f)};
    int pointsOnCenter{0};

    distance -= speed.speed * 5.f * mFT;
    curveOffset += std::sin(curve.speed / 60.f * mFT);

    for(sf::Vector2f& vp : Collisions_vertexPositions)
    {
        if(std::abs(vp.x - mCenterPos.x) <= radius &&
            std::abs(vp.y - mCenterPos.y) <= radius)
        {
            ++pointsOnCenter;
        }
    }

    if(pointsOnCenter > 3)
    {
        killed = true;
    }
}

void CWall::setHueMod(float mHueMod) noexcept
{
    hueMod = mHueMod;
}

[[nodiscard]] SpeedData& CWall::getSpeed() noexcept
{
    return speed;
}

[[nodiscard]] SpeedData& CWall::getCurve() noexcept
{
    return curve;
}

[[nodiscard]] bool CWall::isOverlapping(
    const sf::Vector2f& mPoint) const noexcept
{
    return ssvs::isPointInPolygon(Collisions_vertexPositions, mPoint);
}

} // namespace hg

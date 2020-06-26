// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CPlayer.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

using namespace sf;
using namespace ssvs;

namespace hg
{

CWall::CWall(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos,
    int mSide, float mThickness, float mDistance, const StyleData& styleData,
    const LevelStatus& levelStatus, const SpeedData& mSpeed,
    const SpeedData& mCurve)
    : side{mSide}, initialSides{mHexagonGame.getSides()}, speed{mSpeed}, curve{mCurve}, distance{mDistance}, thickness{mThickness}
{

}

void CWall::draw(HexagonGame& mHexagonGame)
{
    Color colorMain(mHexagonGame.getColorMain());

    if(hueMod != 0)
    {
        colorMain = Utils::transformHue(colorMain, hueMod);
    }
    auto const fieldPos{mHexagonGame.getFieldPos()};
    auto const status{mHexagonGame.getStatus()};
    auto const styleData{mHexagonGame.getStyleData()};
    auto const levelStatus{mHexagonGame.getLevelStatus()};
    auto fieldAngle = ssvu::toRad(styleData.BGRotOff+levelStatus.rotation);
    const float div{ssvu::tau / initialSides * 0.5f};
    const float col_angle{div * 2.f * side};
    const float angle{fieldAngle + div * 2.f * side};

    const sf::Vector2f skewEffect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D,
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D
    };
    const sf::Vector2f skew{1.f, 1.f+skewEffect.y};


    const float radius{mHexagonGame.getRadius() * 0.75f};
    const auto _distance{ssvu::getClampedMin(distance, radius)};
    const auto _distanceThiccL{ssvu::getClampedMin(distance + thickness + mHexagonGame.getWallSkewLeft(), radius)};
    const auto _distanceThiccR{ssvu::getClampedMin(distance + thickness + mHexagonGame.getWallSkewRight(), radius)};
    //For calculating collisions and whatever
    Color colorDebug(255, 0, 0, 150);
    Collisions_vertexPositions[0] = {fieldPos.x + std::cos(col_angle - div) * _distance,
                                     fieldPos.y + std::sin(col_angle - div) * _distance};

    Collisions_vertexPositions[1] = {fieldPos.x + std::cos(col_angle + div) * _distance,
                                     fieldPos.y + std::sin(col_angle + div) * _distance};

    Collisions_vertexPositions[2] = {fieldPos.x + std::cos(col_angle + div + mHexagonGame.getWallAngleLeft()) * _distanceThiccL,
                                     fieldPos.y + std::sin(col_angle + div + mHexagonGame.getWallAngleLeft()) * _distanceThiccL};

    Collisions_vertexPositions[3] = {fieldPos.x + std::cos(col_angle - div + mHexagonGame.getWallAngleRight()) * _distanceThiccR,
                                     fieldPos.y + std::sin(col_angle - div + mHexagonGame.getWallAngleRight()) * _distanceThiccR};

    //For drawing
    vertexPositions[0] = {fieldPos.x + std::cos(angle - div) * _distance,
                          fieldPos.y + std::sin(angle - div) * (_distance/skew.y)};

    vertexPositions[1] = {fieldPos.x + std::cos(angle + div) * _distance,
                          fieldPos.y + std::sin(angle + div) * (_distance/skew.y)};

    vertexPositions[2] = {fieldPos.x + std::cos(angle + div + mHexagonGame.getWallAngleLeft()) * _distanceThiccL,
                          fieldPos.y + std::sin(angle + div + mHexagonGame.getWallAngleLeft()) * (_distanceThiccL/skew.y)};

    vertexPositions[3] = {fieldPos.x + std::cos(angle - div + mHexagonGame.getWallAngleRight()) * _distanceThiccR,
                          fieldPos.y + std::sin(angle - div + mHexagonGame.getWallAngleRight()) * (_distanceThiccR/skew.y)};
    //*/
    mHexagonGame.wallDebugQuads.reserve_more(4);
    mHexagonGame.wallDebugQuads.batch_unsafe_emplace_back(colorDebug,
                                                     Collisions_vertexPositions[0],
                                                     Collisions_vertexPositions[1],
                                                     Collisions_vertexPositions[2],
                                                     Collisions_vertexPositions[3]);
    mHexagonGame.wallQuads.reserve_more(4);
    mHexagonGame.wallQuads.batch_unsafe_emplace_back(colorMain,
        vertexPositions[0],
        vertexPositions[1],
        vertexPositions[2],
        vertexPositions[3]);
}

void CWall::update(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos, FT mFT)
{
    speed.update(mFT);
    curve.update(mFT);

    const float radius{mHexagonGame.getRadius() * 0.75f};
    int pointsOnCenter{0};

    for(sf::Vector2f& vp : Collisions_vertexPositions)
    {
        if(std::abs(vp.x - mCenterPos.x) < radius &&
           std::abs(vp.y - mCenterPos.y) < radius)
        {
            ++pointsOnCenter;
        }
        else
        {
            //TODO:
            //moveTowards(vp, mCenterPos, speed.speed * 5.f * mFT);
            distance -= speed.speed * mFT;
            //rotateRadAround(vp, mCenterPos, curve.speed / 60.f * mFT);
        }
    }

    if(pointsOnCenter > 3)
    {
        killed = true;
    }
}

} // namespace hg

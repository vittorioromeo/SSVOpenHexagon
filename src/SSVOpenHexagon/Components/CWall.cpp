// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Components/CWall.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"

namespace hg {

CWall::CWall(const unsigned int sides, const float wallAngleLeft,
    const float wallAngleRight, const float wallSkewLeft,
    const float wallSkewRight, const sf::Vector2f& centerPos, const int side,
    const float thickness, const float distance, const SpeedData& speed,
    const SpeedData& curve, const float hueMod)
    : _speed{speed}, _curve{curve}, _hueMod{hueMod}, _killed{false}
{
    const float div{ssvu::tau / sides * 0.5f};
    const float angle{div * 2.f * side};

    _vertexPositions[0] = ssvs::getOrbitRad(centerPos, angle - div, distance);
    _vertexPositions[1] = ssvs::getOrbitRad(centerPos, angle + div, distance);
    _vertexPositions[2] = ssvs::getOrbitRad(centerPos,
        angle + div + wallAngleLeft, distance + thickness + wallSkewLeft);
    _vertexPositions[3] = ssvs::getOrbitRad(centerPos,
        angle - div + wallAngleRight, distance + thickness + wallSkewRight);
}

void CWall::draw(sf::Color color, Utils::FastVertexVectorQuads& wallQuads)
{
    if(_hueMod != 0)
    {
        color = Utils::transformHue(color, _hueMod);
    }

    wallQuads.batch_unsafe_emplace_back(color, _vertexPositions[0],
        _vertexPositions[1], _vertexPositions[2], _vertexPositions[3]);
}

void CWall::update(const float wallSpawnDist, const float radius,
    const sf::Vector2f& centerPos, const ssvu::FT ft)
{
    _speed.update(ft);
    _curve.update(ft);

    moveTowardsCenter(wallSpawnDist, radius, centerPos, ft);
    if(_curve._speed != 0.f)
    {
        moveCurve(centerPos, ft);
    }
}

void CWall::moveTowardsCenter(const float wallSpawnDist, const float radius,
    const sf::Vector2f& centerPos, const ssvu::FT ft)
{
    const float halfRadius{radius * 0.5f};
    const float outerBounds{wallSpawnDist * 1.1f};

    int pointsOutOfBounds{0};
    int pointsOnCenter{0};

    for(sf::Vector2f& vp : _vertexPositions)
    {
        const float xDistance = std::abs(vp.x - centerPos.x);
        const float yDistance = std::abs(vp.y - centerPos.y);

        if(xDistance < halfRadius && yDistance < halfRadius)
        {
            ++pointsOnCenter;
            continue;
        }

        if(xDistance > outerBounds || yDistance > outerBounds)
        {
            ++pointsOutOfBounds;
        }

        ssvs::moveTowards(vp, centerPos, _speed._speed * 5.f * ft);
    }

    if(pointsOnCenter == 4 || pointsOutOfBounds == 4)
    {
        _killed = true;
    }
}

void CWall::moveCurve(const sf::Vector2f& centerPos, const ssvu::FT ft)
{
    for(sf::Vector2f& vp : _vertexPositions)
    {
        moveVertexAlongCurve(vp, centerPos, ft);
    }
}

void CWall::setHueMod(float hueMod) noexcept
{
    _hueMod = hueMod;
}

} // namespace hg

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/SpeedData.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SSVStart/Utils/Vector2.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>
#include <cstdint>

namespace hg
{


class CWall
{
private:
    std::array<sf::Vector2f, 4> _vertexPositions;

    SpeedData _speed;
    SpeedData _curve;

    float _hueMod;
    bool _killed;

    void moveTowardsCenter(const float wallSpawnDist, const float radius,
        const sf::Vector2f& centerPos, const ssvu::FT ft);

    void moveCurve(const sf::Vector2f& centerPos, const ssvu::FT ft);

public:
    explicit CWall(const unsigned int sides, const float wallAngleLeft,
        const float wallAngleRight, const float wallSkewLeft,
        const float wallSkewRight, const sf::Vector2f& centerPos,
        const int side, const float thickness, const float distance,
        const SpeedData& speed, const SpeedData& curve);

    void update(const float wallSpawnDist, const float radius,
        const sf::Vector2f& centerPos, const ssvu::FT ft);

    [[gnu::always_inline]] void moveVertexAlongCurve(sf::Vector2f& vertex,
        const sf::Vector2f& centerPos, const ssvu::FT ft) const
    {
        ssvs::rotateRadAround(vertex, centerPos, _curve._speed / 60.f * ft);
    }

    void draw(sf::Color color, Utils::FastVertexVectorQuads& wallQuads);

    void setHueMod(float hueMod) noexcept;

    [[gnu::always_inline, nodiscard]] const std::array<sf::Vector2f, 4>&
    getVertexPositions() const noexcept
    {
        return _vertexPositions;
    }

    [[gnu::always_inline, nodiscard]] const SpeedData& getSpeed() const noexcept
    {
        return _speed;
    }

    [[gnu::always_inline, nodiscard]] const SpeedData& getCurve() const noexcept
    {
        return _curve;
    }

    [[gnu::always_inline, nodiscard]] bool isOverlapping(
        const sf::Vector2f& point) const noexcept
    {
        return Utils::pointInPolygon(_vertexPositions, point.x, point.y);
    }

    [[gnu::always_inline, nodiscard]] constexpr bool
    isCustomWall() const noexcept
    {
        return false;
    }

    [[gnu::always_inline, nodiscard]] std::uint8_t
    getKillingSide() const noexcept
    {
        return 0u;
    }

    [[gnu::always_inline, nodiscard]] bool isDead() const noexcept
    {
        return _killed;
    }
};

} // namespace hg

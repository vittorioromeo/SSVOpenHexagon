// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/SpeedData.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"

#include <SFML/System/Vector2.hpp>

#include <array>

namespace hg
{

class HexagonGame;

class CWall
{
private:
    std::array<sf::Vector2f, 4> vertexPositions;

    SpeedData speed;
    SpeedData curve;

    float hueMod;
    bool killed;

    struct VertexStatus
    {
        sf::Vector2f& vertex;
        bool onCenter{false};

        VertexStatus(sf::Vector2f& mVertex) : vertex{mVertex}
        {
        }
    };
    std::array<VertexStatus, 4> vertexStatuses{{
        {vertexPositions[0]}, {vertexPositions[1]},
        {vertexPositions[2]}, {vertexPositions[3]}
    }};

public:
    CWall(HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos, int mSide,
        float mThickness, float mDistance, const SpeedData& mSpeed,
        const SpeedData& mCurve);

    CWall(const CWall& other)
    {
        vertexPositions = other.vertexPositions;
        speed = other.speed;
        curve = other.curve;
        hueMod = other.hueMod;
        killed = other.killed;
        for(int i = 0; i < 4; ++i)
        {
            vertexStatuses[i].vertex = other.vertexStatuses[i].vertex;
            vertexStatuses[i].onCenter = other.vertexStatuses[i].onCenter;
        }
    }

    CWall& operator = (const CWall&& other)
    {
        vertexPositions = other.vertexPositions;
        speed = other.speed;
        curve = other.curve;
        hueMod = other.hueMod;
        killed = other.killed;
        for(int i = 0; i < 4; ++i)
        {
            vertexStatuses[i].vertex = other.vertexStatuses[i].vertex;
            vertexStatuses[i].onCenter = other.vertexStatuses[i].onCenter;
        }

        return *this;
    }

    void update(const HexagonGame& mHexagonGame, const ssvu::FT mFT);

    void moveTowardsCenter(HexagonGame& mHexagonGame,
        const sf::Vector2f& mCenterPos, const ssvu::FT mFT);

    void moveCurve(const HexagonGame& mHexagonGame, const sf::Vector2f& mCenterPos,
        const ssvu::FT mFT);

    void draw(HexagonGame& mHexagonGame);

    void setHueMod(float mHueMod) noexcept;

    [[gnu::always_inline, nodiscard]]
    const std::array<sf::Vector2f, 4>& getVertexes() const noexcept
    {
        return vertexPositions;
    }

    [[gnu::always_inline, nodiscard]] const SpeedData& getSpeed() const noexcept
    {
        return speed;
    }

    [[gnu::always_inline, nodiscard]] const SpeedData& getCurve() const noexcept
    {
        return curve;
    }

    [[gnu::always_inline, nodiscard]]
    bool isOverlapping(const sf::Vector2f& mPoint) const noexcept
    {
        return Utils::pointInPolygon(vertexPositions, mPoint.x, mPoint.y);
    }

    [[gnu::always_inline, nodiscard]] bool isDead() const noexcept
    {
        return killed;
    }
};

} // namespace hg

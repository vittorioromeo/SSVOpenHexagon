// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <array>
#include <bitset>

namespace hg
{

class HexagonGame;

class CCustomWall
{
public:
    using Handle = int;

private:
    std::array<sf::Vector2f, 4> vertexPositions;
    std::array<sf::Vector2f, 4> oldVertexPositions;
    std::array<sf::Color, 4> vertexColors;
    unsigned int killingSide{0u};

    enum CWFlags : unsigned int
    {
        Collision,
        Deadly,
        CWFlagsCount
    };
    std::bitset<CWFlags::CWFlagsCount> flags{1}; // collision on

    // TODO: Implement this in drawing logic
    // int8_t renderOrder{1};

public:
    CCustomWall();

    void update(HexagonGame& mHexagonGame, ssvu::FT mFT);
    void draw(HexagonGame& mHexagonGame);

    [[gnu::always_inline, nodiscard]] bool isOverlapping(
        const sf::Vector2f& mPoint) const noexcept
    {
        return Utils::pointInPolygon(vertexPositions, mPoint.x, mPoint.y);
    }

    [[gnu::always_inline]] void setVertexPos(
        const int vertexIndex, const sf::Vector2f& pos) noexcept
    {
        oldVertexPositions[vertexIndex] = vertexPositions[vertexIndex];
        vertexPositions[vertexIndex] = pos;
    }

    [[gnu::always_inline]] void setVertexColor(
        const int vertexIndex, const sf::Color& color) noexcept
    {
        vertexColors[vertexIndex] = color;
    }

    [[gnu::always_inline]] void setCanCollide(const bool collide) noexcept
    {
        flags[CWFlags::Collision] = collide;
    }

    [[gnu::always_inline]] void setDeadly(const bool deadly) noexcept
    {
        flags[CWFlags::Deadly] = deadly;
    }

    [[gnu::always_inline]] void setDeadly(const bool deadly) noexcept
    {
        isDeadly = deadly;
    }

    // [[gnu::always_inline]] void setRenderOrder(const int8_t order) noexcept
    // {
    //     renderOrder = order;
    // }

    [[gnu::always_inline, nodiscard]] const sf::Vector2f& getVertexPos(
        const int vertexIndex) const noexcept
    {
        return vertexPositions[vertexIndex];
    }

    [[gnu::always_inline, nodiscard]] const std::array<sf::Vector2f, 4>&
    getVertexPositions() const noexcept
    {
        return vertexPositions;
    }

    [[gnu::always_inline, nodiscard]] const std::array<sf::Vector2f, 4>&
    getOldVertexPositions() const noexcept
    {
        return oldVertexPositions;
    }

    [[gnu::always_inline, nodiscard]] bool getCanCollide() const noexcept
    {
        return flags[CWFlags::Collision];
    }

    [[gnu::always_inline, nodiscard]] bool getDeadly() const noexcept
    {
        return flags[CWFlags::Deadly];
    }

    [[gnu::always_inline, nodiscard]] bool isCustomWall() const noexcept
    {
        return true;
    }

    [[gnu::always_inline]] void setKillingSide(const unsigned int side) noexcept
    {
        killingSide = side;
    }

    [[gnu::always_inline, nodiscard]] unsigned int
    getKillingSide() const noexcept
    {
        return killingSide;
    }

    [[gnu::always_inline, nodiscard]] bool getDeadly() const noexcept
    {
        return isDeadly;
    }

    [[gnu::always_inline, nodiscard]] bool isCustomWall() const noexcept
    {
        return true;
    }
};

} // namespace hg

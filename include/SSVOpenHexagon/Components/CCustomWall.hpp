// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"
#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"

#include <SSVUtils/Core/Common/Frametime.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <array>
#include <bitset>
#include <cstdint>
#include <utility>

namespace hg {

class CCustomWall
{
public:
    using Handle = int;

private:
    std::array<sf::Vector2f, 4> _vertexPositions;
    std::array<sf::Vector2f, 4> _oldVertexPositions;
    std::array<sf::Color, 4> _vertexColors;
    std::uint8_t _killingSide{0u};

    enum CWFlags : unsigned int
    {
        NoCollision,
        Deadly,

        CWFlagsCount
    };

    std::bitset<CWFlags::CWFlagsCount> _flags; // Default: collides, not deadly

public:
    [[gnu::always_inline]] void reset()
    {
        _killingSide = 0u;
        _flags.reset();
    }

    [[gnu::always_inline]] void draw(Utils::FastVertexVectorQuads& wallQuads)
    {
        wallQuads.unsafe_emplace_back(_vertexPositions[0], _vertexColors[0]);
        wallQuads.unsafe_emplace_back(_vertexPositions[1], _vertexColors[1]);
        wallQuads.unsafe_emplace_back(_vertexPositions[2], _vertexColors[2]);
        wallQuads.unsafe_emplace_back(_vertexPositions[3], _vertexColors[3]);
    }

    [[gnu::always_inline, nodiscard]] bool isOverlapping(
        const sf::Vector2f& point) const noexcept
    {
        return Utils::pointInPolygon(_vertexPositions, point.x, point.y);
    }

    [[gnu::always_inline]] void setVertexPos(
        const int vertexIndex, const sf::Vector2f& pos) noexcept
    {
        _oldVertexPositions[vertexIndex] =
            std::exchange(_vertexPositions[vertexIndex], pos);
    }

    [[gnu::always_inline]] void moveVertexPos(
        const int vertexIndex, const sf::Vector2f& offset) noexcept
    {
        _oldVertexPositions[vertexIndex] = _vertexPositions[vertexIndex];
        _vertexPositions[vertexIndex] += offset;
    }

    [[gnu::always_inline]] void moveVertexPos4Same(
        const sf::Vector2f& offset) noexcept
    {
        _oldVertexPositions = _vertexPositions;

        for(sf::Vector2f& v : _vertexPositions)
        {
            v += offset;
        }
    }

    [[gnu::always_inline]] void setVertexColor(
        const int vertexIndex, const sf::Color& color) noexcept
    {
        _vertexColors[vertexIndex] = color;
    }

    [[gnu::always_inline]] void setCanCollide(const bool collide) noexcept
    {
        _flags[CWFlags::NoCollision] = !collide;
    }

    [[gnu::always_inline]] void setDeadly(const bool deadly) noexcept
    {
        _flags[CWFlags::Deadly] = deadly;
    }

    [[gnu::always_inline, nodiscard]] const sf::Vector2f& getVertexPos(
        const int vertexIndex) const noexcept
    {
        return _vertexPositions[vertexIndex];
    }

    [[gnu::always_inline, nodiscard]] const std::array<sf::Vector2f, 4>&
    getVertexPositions() const noexcept
    {
        return _vertexPositions;
    }

    [[gnu::always_inline, nodiscard]] const std::array<sf::Vector2f, 4>&
    getOldVertexPositions() const noexcept
    {
        return _oldVertexPositions;
    }

    [[gnu::always_inline, nodiscard]] bool getCanCollide() const noexcept
    {
        return !_flags[CWFlags::NoCollision];
    }

    [[gnu::always_inline, nodiscard]] bool getDeadly() const noexcept
    {
        return _flags[CWFlags::Deadly];
    }

    [[gnu::always_inline, nodiscard]] constexpr bool
    isCustomWall() const noexcept
    {
        return true;
    }

    [[gnu::always_inline]] void setKillingSide(const std::uint8_t side) noexcept
    {
        _killingSide = side;
    }

    [[gnu::always_inline, nodiscard]] std::uint8_t
    getKillingSide() const noexcept
    {
        return _killingSide;
    }
};

} // namespace hg

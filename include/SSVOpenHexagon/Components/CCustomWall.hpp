// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/FastVertexVector.hpp"
#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"

namespace hg
{

class CCustomWall
{
public:
    using Handle = int;

private:
    sf::base::Array<sf::Vec2f, 4> _vertexPositions;
    sf::base::Array<sf::Vec2f, 4> _oldVertexPositions;
    sf::base::Array<sf::Color, 4> _vertexColors;
    sf::base::U8                  _killingSide{0u};

    enum CWFlags : sf::base::U8
    {
        NoCollision = 1u << 0,
        Deadly      = 1u << 1,
    };

    sf::base::U8 _flags{0u}; // Default: collides, not deadly

public:
    [[gnu::always_inline]] void reset()
    {
        _killingSide = 0u;
        _flags       = 0u;
    }

    [[gnu::always_inline]] void draw(Utils::FastVertexVectorTris& wallQuads)
    {
        wallQuads.unsafe_emplace_back_quad( //
            _vertexPositions[0],
            _vertexColors[0], //
            _vertexPositions[1],
            _vertexColors[1], //
            _vertexPositions[2],
            _vertexColors[2], //
            _vertexPositions[3],
            _vertexColors[3]);
    }

    [[nodiscard, gnu::always_inline]] bool isOverlapping(const sf::Vec2f point) const noexcept
    {
        // Cannot use `pointInFourVertexPolygon` here due to vertex ordering
        // requirements.

        return Utils::pointInPolygon<4>(_vertexPositions, point.x, point.y);
    }

    [[gnu::always_inline]] void setVertexPos(const int vertexIndex, const sf::Vec2f pos) noexcept
    {
        _oldVertexPositions[vertexIndex] = _vertexPositions[vertexIndex];
        _vertexPositions[vertexIndex]    = pos;
    }

    [[gnu::always_inline]] void moveVertexPos(const int vertexIndex, const sf::Vec2f offset) noexcept
    {
        _oldVertexPositions[vertexIndex] = _vertexPositions[vertexIndex];
        _vertexPositions[vertexIndex] += offset;
    }

    [[gnu::always_inline]] void moveVertexPos4Same(const sf::Vec2f offset) noexcept
    {
        _oldVertexPositions = _vertexPositions;

        for (sf::Vec2f& v : _vertexPositions)
        {
            v += offset;
        }
    }

    [[gnu::always_inline]] void setVertexColor(const int vertexIndex, const sf::Color& color) noexcept
    {
        _vertexColors[vertexIndex] = color;
    }

    [[gnu::always_inline]] void setCanCollide(const bool collide) noexcept
    {
        if (collide)
            _flags &= ~CWFlags::NoCollision;
        else
            _flags |= CWFlags::NoCollision;
    }

    [[gnu::always_inline]] void setDeadly(const bool deadly) noexcept
    {
        if (deadly)
            _flags |= CWFlags::Deadly;
        else
            _flags &= ~CWFlags::Deadly;
    }

    [[nodiscard, gnu::always_inline]] const sf::Vec2f getVertexPos(const int vertexIndex) const noexcept
    {
        return _vertexPositions[vertexIndex];
    }

    [[nodiscard, gnu::always_inline]] const sf::base::Array<sf::Vec2f, 4>& getVertexPositions() const noexcept
    {
        return _vertexPositions;
    }

    [[nodiscard, gnu::always_inline]] const sf::base::Array<sf::Vec2f, 4>& getOldVertexPositions() const noexcept
    {
        return _oldVertexPositions;
    }

    [[nodiscard, gnu::always_inline]] bool getCanCollide() const noexcept
    {
        return (_flags & CWFlags::NoCollision) == 0u;
    }

    [[nodiscard, gnu::always_inline]] bool getDeadly() const noexcept
    {
        return (_flags & CWFlags::Deadly) != 0u;
    }

    [[nodiscard, gnu::always_inline]] constexpr bool isCustomWall() const noexcept
    {
        return true;
    }

    [[gnu::always_inline]] void setKillingSide(const sf::base::U8 side) noexcept
    {
        _killingSide = side;
    }

    [[nodiscard, gnu::always_inline]] sf::base::U8 getKillingSide() const noexcept
    {
        return _killingSide;
    }
};

} // namespace hg

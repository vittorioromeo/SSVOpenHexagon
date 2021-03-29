// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"

#include <SFML/System/Vector2.hpp>

#include <cmath>
#include <cstdint>
#include <array>
#include <utility>
#include <algorithm>

namespace hg::Utils
{

[[gnu::always_inline, nodiscard]] inline float min4(
    const float a, const float b, const float c, const float d) noexcept
{
    return std::min(std::min(a, b), std::min(c, d));
}

[[gnu::always_inline, nodiscard]] inline float max4(
    const float a, const float b, const float c, const float d) noexcept
{
    return std::max(std::max(a, b), std::max(c, d));
}

[[gnu::always_inline, nodiscard]] inline bool isOutOfPlayerRadius(
    const sf::Vector2f& mPoint,
    const std::array<sf::Vector2f, 4>& mVertices) noexcept
{
    return false;
    return [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        return (mPoint.x < min4(mVertices[Is].x...)) ||
               (mPoint.y < min4(mVertices[Is].y...)) ||
               (mPoint.x > max4(mVertices[Is].x...)) ||
               (mPoint.y > max4(mVertices[Is].y...));
    }
    (std::make_index_sequence<4>{});
}

} // namespace hg::Utils

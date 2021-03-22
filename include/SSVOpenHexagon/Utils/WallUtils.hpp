// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Utils/PointInPolygon.hpp"

#include <SFML/System/Vector2.hpp>

#include <cmath>

namespace hg::Utils
{

template <typename TC>
[[gnu::always_inline, nodiscard]] inline bool broadphaseManhattan(
    const sf::Vector2f& mCenterPos, const float mRadius,
    const TC& mVertexPositions) noexcept
{
    const float broadRadius{mRadius * 1.2f};

    for(const sf::Vector2f& vp : mVertexPositions)
    {
        if(const float xDistance = std::abs(vp.x - mCenterPos.x);
            xDistance < broadRadius)
        {
            return false;
        }

        if(const float yDistance = std::abs(vp.y - mCenterPos.y);
            yDistance < broadRadius)
        {
            return false;
        }
    }

    return true;
}

template <typename TC>
[[gnu::always_inline, nodiscard]] inline bool narrowphaseOverlap(
    const bool mOutOfPlayerRadius, const sf::Vector2f& mPoint,
    const TC& mVertexPositions) noexcept
{
    if(mOutOfPlayerRadius)
    {
        return false;
    }

    return Utils::pointInPolygon(mVertexPositions, mPoint.x, mPoint.y);
}

} // namespace hg::Utils

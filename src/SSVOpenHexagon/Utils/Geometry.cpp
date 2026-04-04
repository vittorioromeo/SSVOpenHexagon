// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Geometry.hpp"

#include <SFML/Base/Math/Sqrt.hpp>
#include <SFML/System/Vec2.hpp>

namespace hg::Utils
{

[[nodiscard]] unsigned int getLineCircleIntersection(
    sf::Vec2f&      i1,
    sf::Vec2f&      i2,
    const sf::Vec2f p1,
    const sf::Vec2f p2,
    const float     mRadiusSquared)
{
    const float dx{p2.x - p1.x};
    const float dy{p2.y - p1.y};
    const float a{dx * dx + dy * dy};
    const float b{2.f * (dx * p1.x + dy * p1.y)};
    const float c{p1.x * p1.x + p1.y * p1.y - mRadiusSquared};
    const float delta{b * b - 4.f * a * c};

    // No intersections.
    if (delta < 0.f)
    {
        return 0u;
    }

    float       t;
    const float twoA{2.f * a};

    // One intersection.
    if (delta < epsilon)
    {
        t  = -b / twoA;
        i1 = {p1.x + t * dx, p1.y + t * dy};
        return 1u;
    }

    // Two intersections.
    const float sqrtDelta{SFML_BASE_MATH_SQRTF(delta)};
    t  = (-b + sqrtDelta) / twoA;
    i1 = {p1.x + t * dx, p1.y + t * dy};
    t  = (-b - sqrtDelta) / twoA;
    i2 = {p1.x + t * dx, p1.y + t * dy};
    return 2u;
}

[[nodiscard]] bool getLineCircleClosestIntersection(
    sf::Vec2f&      mIntersection,
    const sf::Vec2f mPos,
    const sf::Vec2f p1,
    const sf::Vec2f p2,
    const float     mRadiusSquared)
{
    sf::Vec2f v1, v2;

    switch (getLineCircleIntersection(v1, v2, p1, p2, mRadiusSquared))
    {
        case 1u:
            mIntersection = v1;
            return true;

        case 2u:
            if ((v1 - mPos).lengthSquared() > (v2 - mPos).lengthSquared())
            {
                mIntersection = v2;
            }
            else
            {
                mIntersection = v1;
            }
            return true;

        default:
            return false;
    }
}

} // namespace hg::Utils

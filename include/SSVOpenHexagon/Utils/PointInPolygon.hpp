// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/System/Vec2.hpp>
#include <SFML/Base/SizeT.hpp>

namespace hg::Utils {

template <sf::base::SizeT N, typename TC, typename T>
[[gnu::always_inline, gnu::pure, nodiscard]] inline bool pointInPolygon(
    const TC& mVertices, const T x, const T y) noexcept
{
    bool result{false};

    for (sf::base::SizeT i{0}, j{N - 1}; i < N; j = i++)
    {
        const auto& vI{mVertices[i]};
        const auto& vJ{mVertices[j]};

        if (((vI.y > y) != (vJ.y > y)) &&
            (x < (vJ.x - vI.x) * (y - vI.y) / (vJ.y - vI.y) + vI.x))
        {
            result = !result;
        }
    }

    return result;
}


[[gnu::always_inline, gnu::pure, nodiscard]] inline bool
pointInFourVertexPolygon(const sf::Vec2f& a, const sf::Vec2f& b,
    const sf::Vec2f& c, const sf::Vec2f& d, const sf::Vec2f& point) noexcept
{
    const sf::Vec2f ab = b - a;
    const sf::Vec2f bc = c - b;
    const sf::Vec2f cd = d - c;
    const sf::Vec2f da = a - d;

    const sf::Vec2f ap_ab = point - a;
    const sf::Vec2f bp_bc = point - b;
    const sf::Vec2f cp_cd = point - c;
    const sf::Vec2f dp_da = point - d;

    const float ab_x_ap = ab.cross(ap_ab);
    const float bc_x_bp = bc.cross(bp_bc);
    const float cd_x_cp = cd.cross(cp_cd);
    const float da_x_dp = da.cross(dp_da);

    return (ab_x_ap <= 0.f && bc_x_bp <= 0.f && cd_x_cp <= 0.f &&
               da_x_dp <= 0.f) ||
           (ab_x_ap >= 0.f && bc_x_bp >= 0.f && cd_x_cp >= 0.f &&
               da_x_dp >= 0.f);
}

} // namespace hg::Utils

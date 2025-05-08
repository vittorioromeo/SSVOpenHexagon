// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/System/Vec2.hpp>

namespace hg::Utils {

inline constexpr float epsilon{1.0e-4};

[[nodiscard]] unsigned int getLineCircleIntersection(sf::Vec2f& i1,
    sf::Vec2f& i2, const sf::Vec2f& p1, const sf::Vec2f& p2,
    const float mRadiusSquared);

[[nodiscard]] bool getLineCircleClosestIntersection(sf::Vec2f& mIntersection,
    const sf::Vec2f& mPos, const sf::Vec2f& p1, const sf::Vec2f& p2,
    const float mRadiusSquared);

} // namespace hg::Utils

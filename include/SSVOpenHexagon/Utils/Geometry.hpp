// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/System/Vector2.hpp>

namespace hg::Utils {

inline constexpr float epsilon{1.0e-4};

[[nodiscard]] unsigned int getLineCircleIntersection(sf::Vector2f& i1,
    sf::Vector2f& i2, const sf::Vector2f& p1, const sf::Vector2f& p2,
    const float mRadiusSquared);

[[nodiscard]] bool getLineCircleClosestIntersection(sf::Vector2f& mIntersection,
    const sf::Vector2f& mPos, const sf::Vector2f& p1, const sf::Vector2f& p2,
    const float mRadiusSquared);

} // namespace hg::Utils

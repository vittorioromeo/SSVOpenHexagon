// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vec2.hpp>

namespace hg::Utils {

struct ViewTransform
{
    sf::Vec2f skew{1.f, 1.f};
};

[[nodiscard]] inline sf::View computeCameraView(
    const sf::View& baseView,
    const ViewTransform& transform = ViewTransform{}) noexcept
{
    sf::View result = baseView;
    result.size = {baseView.size.x * transform.skew.x,
        baseView.size.y * transform.skew.y};
    return result;
}

} // namespace hg::Utils

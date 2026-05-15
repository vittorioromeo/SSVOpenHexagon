// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Graphics/View.hpp"

#include "SFML/System/Vec2.hpp"

namespace hg::Utils
{

struct ViewTransform
{
    sf::Vec2f skew{1.f, 1.f};
};

[[nodiscard]] inline sf::View computeCameraView(const sf::View& baseView, const ViewTransform& transform = ViewTransform{}) noexcept
{
    sf::View result = baseView;
    result.size     = {baseView.size.x * transform.skew.x, baseView.size.y * transform.skew.y};

    // SFML's `View::getTransform()` asserts that `size.{x,y}` are non-zero
    // when a draw using this view is flushed. A degenerate skew (e.g. a
    // level style whose `_3dSkew` drives `1 + effect` to <= 0) used to
    // crash the preview pipeline silently because the bad view rode
    // inside a deferred draw batch and only the flush at `display()`
    // surfaced the assert. Floor each axis to a tiny positive value so
    // the assert can't fire from this path again -- the resulting view
    // is geometrically degenerate but won't trip an SFML invariant, and
    // the actual scaling source has its own clamp.
    constexpr float kMinSize = 0.001f;
    if (result.size.x < kMinSize)
        result.size.x = kMinSize;
    if (result.size.y < kMinSize)
        result.size.y = kMinSize;

    return result;
}

} // namespace hg::Utils

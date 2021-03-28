// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Graphics/Color.hpp>

#include <cmath>

namespace hg::Utils
{

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Color getColorDarkened(
    sf::Color mColor, const float mMultiplier)
{
    mColor.r /= mMultiplier;
    mColor.b /= mMultiplier;
    mColor.g /= mMultiplier;
    return mColor;
}

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Color transformHue(
    const sf::Color& in, const float H)
{
    const float u{std::cos(H * 3.14f / 180.f)};
    const float w{std::sin(H * 3.14f / 180.f)};

    sf::Color ret;
    ret.r = (.701 * u + .168 * w) * in.r + (-.587 * u + .330 * w) * in.g +
            (-.114 * u - .497 * w) * in.b;
    ret.g = (-.299 * u - .328 * w) * in.r + (.413 * u + .035 * w) * in.g +
            (-.114 * u + .292 * w) * in.b;
    ret.b = (-.3 * u + 1.25 * w) * in.r + (-.588 * u - 1.05 * w) * in.g +
            (.886 * u - .203 * w) * in.b;
    return ret;
}

} // namespace hg::Utils

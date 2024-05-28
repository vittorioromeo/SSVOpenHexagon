// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Glsl.hpp>

#include <cmath>
#include <cstdint>

namespace hg::Utils {

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Color getColorDarkened(
    sf::Color mColor, const float mMultiplier) noexcept
{
    if (mMultiplier != 0.f)
    {
        mColor.r = static_cast<float>(mColor.r) / mMultiplier;
        mColor.b = static_cast<float>(mColor.b) / mMultiplier;
        mColor.g = static_cast<float>(mColor.g) / mMultiplier;
    }

    return mColor;
}

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Color transformHue(
    const sf::Color& in, const float H) noexcept
{
    const float u{std::cos(H * 3.14f / 180.f)};
    const float w{std::sin(H * 3.14f / 180.f)};

    return sf::Color{
        //
        static_cast<std::uint8_t>((.701 * u + .168 * w) * in.r +
                                  (-.587 * u + .330 * w) * in.g +
                                  (-.114 * u - .497 * w) * in.b),

        static_cast<std::uint8_t>((-.299 * u - .328 * w) * in.r +
                                  (.413 * u + .035 * w) * in.g +
                                  (-.114 * u + .292 * w) * in.b),

        static_cast<std::uint8_t>((-.3 * u + 1.25 * w) * in.r +
                                  (-.588 * u - 1.05 * w) * in.g +
                                  (.886 * u - .203 * w) * in.b),

        static_cast<std::uint8_t>(255) //
    };
}

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Color getColorFromHue(
    const float hue) noexcept
{
    const int i = std::floor(hue * 6.f);

    const float f = (hue * 6.f) - i;
    const float q = 1.f - f;
    const float t = f;

    const auto ret = [](const float r, const float g, const float b)
    { return sf::Color(r * 255.f, g * 255.f, b * 255.f); };

    switch (i)
    {
        case 0: return ret(1.f, t, 0.f);
        case 1: return ret(q, 1.f, 0.f);
        case 2: return ret(0.f, 1.f, t);
        case 3: return ret(0.f, q, 1.f);
        case 4: return ret(t, 0.f, 1.f);
    }

    return ret(1.f, 0.f, q);
}

[[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr std::uint8_t
componentClamp(const float value) noexcept
{
    if (value > 255.f)
    {
        return std::uint8_t(255);
    }

    if (value < 0.f)
    {
        return std::uint8_t(0);
    }

    return static_cast<std::uint8_t>(value);
}

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Glsl::Vec3 toGLSLVec3(
    const sf::Color& color) noexcept
{
    return {//
        static_cast<float>(color.r) / 255.f,
        static_cast<float>(color.g) / 255.f,
        static_cast<float>(color.b) / 255.f};
}

[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Glsl::Vec4 toGLSLVec4(
    const sf::Color& color) noexcept
{
    return {//
        static_cast<float>(color.r) / 255.f,
        static_cast<float>(color.g) / 255.f,
        static_cast<float>(color.b) / 255.f,
        static_cast<float>(color.a) / 255.f};
}

} // namespace hg::Utils

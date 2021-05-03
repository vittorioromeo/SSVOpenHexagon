// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Json {

class Value;

}

namespace ssvuj {

using Obj = Json::Value;

}

namespace hg {

struct PulseColor
{
    int r;
    int g;
    int b;
    int a;
};

[[nodiscard]] PulseColor pulse_from_json(const ssvuj::Obj& root) noexcept;

struct ColorData
{
    bool main{};
    bool dynamic{};
    bool dynamicOffset{};
    float dynamicDarkness{};
    float hueShift{};
    float offset{};

    sf::Color color{};
    PulseColor pulse{};

    explicit ColorData();

    explicit ColorData(const ssvuj::Obj& mRoot);

    explicit ColorData(const bool mMain, const bool mDynamic,
        const bool mDynamicOffset, const float mDynamicDarkness,
        const float mHueShift, const float mOffset, sf::Color mColor,
        const PulseColor& mPulse);
};

} // namespace hg

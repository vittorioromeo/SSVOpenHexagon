// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"

#include <SFML/Graphics/Color.hpp>

struct ColorData
{
    bool main, dynamic, dynamicOffset;
    float dynamicDarkness, hueShift, offset;
    sf::Color color, pulse;

    ColorData() = default;

    ColorData(const ssvuj::Obj& mRoot)
        : main{ssvuj::getExtr<bool>(mRoot, "main", false)},
          dynamic{ssvuj::getExtr<bool>(mRoot, "dynamic", false)},
          dynamicOffset{ssvuj::getExtr<bool>(mRoot, "dynamic_offset", false)},
          dynamicDarkness{
              ssvuj::getExtr<float>(mRoot, "dynamic_darkness", 1.f)},
          hueShift{ssvuj::getExtr<float>(mRoot, "hue_shift", 0.f)},
          offset{ssvuj::getExtr<float>(mRoot, "offset", 0.f)},
          color{ssvuj::getExtr<sf::Color>(mRoot, "value", sf::Color::White)},
          pulse{ssvuj::getExtr<sf::Color>(mRoot, "pulse", sf::Color::White)} {};

    ColorData(const bool mMain, const bool mDynamic, const bool mDynamicOffset,
        const float mDynamicDarkness, const float mHueShift,
        const float mOffset, sf::Color mColor, sf::Color mPulse)
        : main{mMain}, dynamic{mDynamic}, dynamicOffset{mDynamicOffset},
          dynamicDarkness{mDynamicDarkness}, hueShift{mHueShift},
          offset{mOffset}, color{mColor}, pulse{mPulse}
    {
    }
};

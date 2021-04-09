// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ColorData.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"

#include <string>

namespace hg {

ColorData::ColorData() = default;

ColorData::ColorData(const ssvuj::Obj& mRoot)
    : main{ssvuj::getExtr<bool>(mRoot, "main", false)},
      dynamic{ssvuj::getExtr<bool>(mRoot, "dynamic", false)},
      dynamicOffset{ssvuj::getExtr<bool>(mRoot, "dynamic_offset", false)},
      dynamicDarkness{ssvuj::getExtr<float>(mRoot, "dynamic_darkness", 1.f)},
      hueShift{ssvuj::getExtr<float>(mRoot, "hue_shift", 0.f)},
      offset{ssvuj::getExtr<float>(mRoot, "offset", 0.f)},
      color{ssvuj::getExtr<sf::Color>(mRoot, "value", sf::Color::Black)},
      pulse{hg::pulse_from_json(mRoot)}
{}

ColorData::ColorData(const bool mMain, const bool mDynamic,
    const bool mDynamicOffset, const float mDynamicDarkness,
    const float mHueShift, const float mOffset, sf::Color mColor,
    const PulseColor& mPulse)
    : main{mMain},
      dynamic{mDynamic},
      dynamicOffset{mDynamicOffset},
      dynamicDarkness{mDynamicDarkness},
      hueShift{mHueShift},
      offset{mOffset},
      color{mColor},
      pulse{mPulse}
{}

[[nodiscard]] PulseColor pulse_from_json(const ssvuj::Obj& root) noexcept
{
    if(!ssvuj::hasObj(root, "pulse"))
    {
        return {0, 0, 0, 255};
    }

    const auto& pulseObj = ssvuj::getObj(root, "pulse");

    return {
        ssvuj::getExtr<int>(pulseObj, 0), //
        ssvuj::getExtr<int>(pulseObj, 1), //
        ssvuj::getExtr<int>(pulseObj, 2), //
        ssvuj::getExtr<int>(pulseObj, 3)  //
    };
}

} // namespace hg

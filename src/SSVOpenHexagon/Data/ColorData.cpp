// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ColorData.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"

#include <string>

namespace hg
{
[[nodiscard]] PulseColor pulse_from_json(const ssvuj::Obj& root) noexcept
{
    if(!ssvuj::hasObj(root, "pulse"))
    {
        return {0, 0, 0, 255};
    }

    const auto& pulseObj = ssvuj::getObj(root, "pulse");

    return {ssvuj::getExtr<int>(pulseObj, 0), ssvuj::getExtr<int>(pulseObj, 1),
        ssvuj::getExtr<int>(pulseObj, 2), ssvuj::getExtr<int>(pulseObj, 3)};
}
} // namespace hg

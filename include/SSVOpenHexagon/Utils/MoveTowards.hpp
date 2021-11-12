// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <algorithm>
#include <cmath>

namespace hg::Utils {

[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr float
getMoveTowards(float value, const float target, const float step) noexcept
{
    SSVOH_ASSERT(step >= 0);

    if(value < target)
    {
        value += step;
        if(value > target)
        {
            value = target;
        }
    }
    else if(value > target)
    {
        value -= step;
        if(value < target)
        {
            value = target;
        }
    }

    return value;
}

[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr float
getMoveTowardsZero(const float value, const float step) noexcept
{
    return getMoveTowards(value, 0.f, step);
}

[[gnu::always_inline]] inline constexpr void moveTowards(
    float& value, const float target, const float step) noexcept
{
    value = getMoveTowards(value, target, step);
}

[[gnu::always_inline]] inline constexpr void moveTowardsZero(
    float& value, const float step) noexcept
{
    value = getMoveTowardsZero(value, step);
}

} // namespace hg::Utils

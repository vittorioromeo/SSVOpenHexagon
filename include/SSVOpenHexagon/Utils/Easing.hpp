// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Utils
{

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSaturated(
    float mValue)
{
    return std::max(0.f, std::min(1.f, mValue));
}

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSmoothStep(
    float edge0, float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * (3 - 2 * x);
}

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSmootherStep(
    float edge0, float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * x * (x * (x * 6 - 15) + 10);
}

} // namespace hg::Utils

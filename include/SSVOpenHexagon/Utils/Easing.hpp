// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Utils {

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSaturated(
    const float mValue)
{
    if(mValue < 0.f)
    {
        return 0.f;
    }

    if(mValue > 1.f)
    {
        return 1.f;
    }

    return mValue;
}

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSmoothStep(
    const float edge0, const float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * (3.f - 2.f * x);
}

[[nodiscard, gnu::pure, gnu::always_inline]] inline float getSmootherStep(
    const float edge0, const float edge1, float x)
{
    x = getSaturated((x - edge0) / (edge1 - edge0));
    return x * x * x * (x * (x * 6.f - 15.f) + 10.f);
}

} // namespace hg::Utils

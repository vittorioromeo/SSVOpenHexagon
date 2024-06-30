// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Utils {

inline constexpr float pi{3.14159265359f};
inline constexpr float tau{6.28318530718f};
inline constexpr float radDegRatio{0.01745329251f};

template <typename T>
[[gnu::always_inline]] inline constexpr T toRad(const T& mX) noexcept
{
    return mX * radDegRatio;
}

template <typename T>
[[gnu::always_inline]] inline constexpr T getSign(const T& mX) noexcept
{
    return (T(0) < mX) - (mX < T(0));
}

} // namespace hg::Utils

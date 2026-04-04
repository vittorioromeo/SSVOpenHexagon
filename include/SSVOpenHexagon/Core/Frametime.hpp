#pragma once

namespace hg
{

/// @brief Default constexpr ratio between seconds and frametime.
constexpr float secondsFTRatio{60.f};

/// @brief Converts frametime to seconds.
template <typename T>
constexpr T getFTToSeconds(T mFT) noexcept
{
    return mFT / secondsFTRatio;
}

/// @brief Converts seconds to frametime.
template <typename T>
constexpr T getSecondsToFT(T mSeconds) noexcept
{
    return mSeconds * secondsFTRatio;
}

/// @brief Converts frametime to frames per seconds.
template <typename T>
constexpr T getFTToFPS(T mFT) noexcept
{
    return secondsFTRatio / mFT;
}

} // namespace hg

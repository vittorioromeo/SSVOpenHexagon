// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg {

// Allow us to represent the game's version in a major.minor.micro format
struct GameVersion
{
    int major;
    int minor;
    int micro;

    [[nodiscard]] constexpr bool operator<(
        const GameVersion& rhs) const noexcept
    {
        if (major != rhs.major)
        {
            return major < rhs.major;
        }

        if (minor != rhs.minor)
        {
            return minor < rhs.minor;
        }

        return micro < rhs.micro;
    }

    [[nodiscard]] constexpr bool operator==(
        const GameVersion& other) const noexcept
    {
        return (major == other.major) && (minor == other.minor) &&
               (micro == other.micro);
    }

    [[nodiscard]] constexpr bool operator!=(
        const GameVersion& other) const noexcept
    {
        return !(*this == other);
    }
};

inline constexpr GameVersion GAME_VERSION{2, 1, 7};
inline constexpr auto& GAME_VERSION_STR = "2.1.7";

} // namespace hg

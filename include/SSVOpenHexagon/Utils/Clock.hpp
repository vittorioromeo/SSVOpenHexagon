// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <chrono>

namespace hg {

using HRClock = std::chrono::high_resolution_clock;
using HRTimePoint = std::chrono::time_point<HRClock>;

[[nodiscard]] inline auto hrSecondsSince(const HRTimePoint tp) noexcept
{
    return std::chrono::duration_cast<std::chrono::seconds>(HRClock::now() - tp)
        .count();
}

} // namespace hg

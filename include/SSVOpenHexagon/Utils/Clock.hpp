// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <chrono>

namespace hg {

using HRClockImpl = std::chrono::high_resolution_clock;
using HRTimePointImpl = std::chrono::time_point<HRClockImpl>;

struct HRClock : HRClockImpl
{
    using HRClockImpl::HRClockImpl;
};

struct HRTimePoint : HRTimePointImpl
{
    using HRTimePointImpl::time_point;

    HRTimePoint(HRTimePointImpl x) : HRTimePointImpl(x)
    {}
};

[[nodiscard]] inline auto hrSecondsSince(const HRTimePoint tp) noexcept
{
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::high_resolution_clock::now() - tp)
        .count();
}

} // namespace hg

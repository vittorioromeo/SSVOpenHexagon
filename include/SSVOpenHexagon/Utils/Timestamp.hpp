// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace hg::Utils {

using SCClock = std::chrono::system_clock;
using SCTimePoint = std::chrono::time_point<SCClock>;

[[nodiscard]] std::uint64_t timestamp(const SCTimePoint tp);

[[nodiscard]] std::uint64_t nowTimestamp();

[[nodiscard]] SCTimePoint toTimepoint(const std::uint64_t timestamp);

[[nodiscard]] std::string formatTimepoint(
    const SCTimePoint time, const std::string& format);

} // namespace hg::Utils

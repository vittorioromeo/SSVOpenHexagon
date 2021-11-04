// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace hg::Utils {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

[[nodiscard]] std::uint64_t timestamp(const TimePoint tp);
[[nodiscard]] std::uint64_t nowTimestamp();
[[nodiscard]] TimePoint toTimepoint(const std::uint64_t timestamp);
[[nodiscard]] std::string formatTimepoint(
    const TimePoint time, const std::string& format);

} // namespace hg::Utils

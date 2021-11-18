// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Timestamp.hpp"

#include <iomanip>
#include <sstream>

namespace hg::Utils {

[[nodiscard]] std::uint64_t timestamp(const SCTimePoint tp)
{
    return std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch())
        .count();
}

[[nodiscard]] std::uint64_t nowTimestamp()
{
    return timestamp(SCClock::now());
}

[[nodiscard]] SCTimePoint toTimepoint(const std::uint64_t timestamp)
{
    return SCTimePoint{} + std::chrono::seconds(timestamp);
}

[[nodiscard]] std::string formatTimepoint(
    const SCTimePoint time, const std::string& format)
{
    const std::time_t tt = std::chrono::system_clock::to_time_t(time);
    const std::tm tm = *std::gmtime(&tt); // GMT (UTC)

    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

} // namespace hg::Utils

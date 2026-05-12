// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/Timestamp.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"

#include <ctime>

namespace hg::Utils
{

[[nodiscard]] sf::base::U64 timestamp(const SCTimePoint tp)
{
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

[[nodiscard]] sf::base::U64 nowTimestamp()
{
    return timestamp(SCClock::now());
}

[[nodiscard]] SCTimePoint toTimepoint(const sf::base::U64 timestamp)
{
    return SCTimePoint{} + std::chrono::seconds(timestamp);
}

[[nodiscard]] sf::base::String formatTimepoint(const SCTimePoint time, const sf::base::String& format)
{
    const std::time_t tt = std::chrono::system_clock::to_time_t(time);
    const std::tm     tm = *std::gmtime(&tt); // GMT (UTC)

    char         buf[128];
    const size_t n = std::strftime(buf, sizeof(buf), format.cStr(), &tm);
    return sf::base::String(buf, n);
}

} // namespace hg::Utils

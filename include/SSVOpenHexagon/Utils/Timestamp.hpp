// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/StdChrono.hpp"

#include <string>

namespace hg::Utils
{

using SCClock     = std::chrono::system_clock;
using SCTimePoint = std::chrono::time_point<SCClock>;

[[nodiscard]] sf::base::U64 timestamp(const SCTimePoint tp);

[[nodiscard]] sf::base::U64 nowTimestamp();

[[nodiscard]] SCTimePoint toTimepoint(const sf::base::U64 timestamp);

[[nodiscard]] std::string formatTimepoint(const SCTimePoint time, const std::string& format);

} // namespace hg::Utils

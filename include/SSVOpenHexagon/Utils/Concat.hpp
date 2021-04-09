// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <sstream>

namespace hg::Utils {

template <typename... Ts>
[[nodiscard]] std::string concat(const Ts&... xs)
{
    thread_local std::ostringstream oss;
    oss.str("");

    (oss << ... << xs);
    return oss.str();
}

} // namespace hg::Utils

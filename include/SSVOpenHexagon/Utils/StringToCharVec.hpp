// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <vector>

namespace hg::Utils {

[[nodiscard]] std::vector<char> stringToCharVec(const std::string& s)
{
    return std::vector<char>(s.begin(), s.end());
}

} // namespace hg::Utils

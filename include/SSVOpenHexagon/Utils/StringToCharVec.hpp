// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/String.hpp"

#include <vector>

namespace hg::Utils
{

[[nodiscard]] inline std::vector<char> stringToCharVec(const sf::base::String& s)
{
    return std::vector<char>(s.data(), s.data() + s.size());
}

} // namespace hg::Utils

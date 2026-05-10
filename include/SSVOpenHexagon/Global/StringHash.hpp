// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/FwdStdHash.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

#include <string_view>

template <>
struct std::hash<sf::base::String>
{
    [[nodiscard]] sf::base::SizeT operator()(const sf::base::String& s) const noexcept
    {
        return std::hash<std::string_view>{}(std::string_view{s.data(), s.size()});
    }
};

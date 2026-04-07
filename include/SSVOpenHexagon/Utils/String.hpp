// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

#include <cctype>

namespace hg::Utils
{

inline void lTrim(sf::base::String& str)
{
    sf::base::SizeT i = 0;
    while (i < str.size() && std::isspace(str[i]))
        ++i;

    if (i > 0)
        str.erase(0, i);
}

inline void rTrim(sf::base::String& str)
{
    sf::base::SizeT i = str.size();
    while (i > 0 && std::isspace(str[i - 1]))
        --i;

    if (i < str.size())
        str.erase(i, str.size() - i);
}

inline void lrTrim(sf::base::String& str)
{
    lTrim(str);
    rTrim(str);
}

[[nodiscard]] inline sf::base::String getLTrim(sf::base::String s)
{
    lTrim(s);
    return s;
}

[[nodiscard]] inline sf::base::String getRTrim(sf::base::String s)
{
    rTrim(s);
    return s;
}

[[nodiscard]] inline sf::base::String getLRTrim(sf::base::String s)
{
    lrTrim(s);
    return s;
}

inline void uppercasify(sf::base::String& s)
{
    for (auto& c : s)
    {
        c = std::toupper(c);
    }
}

[[nodiscard]] inline sf::base::String toUppercase(sf::base::String s)
{
    uppercasify(s);
    return s;
}

inline void lowercasify(sf::base::String& s)
{
    for (auto& c : s)
    {
        c = std::tolower(c);
    }
}

[[nodiscard]] inline sf::base::String toLowercase(sf::base::String s)
{
    lowercasify(s);
    return s;
}

} // namespace hg::Utils

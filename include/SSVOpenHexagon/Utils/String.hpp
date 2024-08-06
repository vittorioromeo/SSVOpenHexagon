// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <SFML/Base/Algorithm.hpp>

namespace hg::Utils {

inline void lTrim(std::string& str)
{
    const auto it = sf::base::findIf(
        str.begin(), str.end(), [](char ch) { return !std::isspace(ch); });

    str.erase(str.begin(), it);
}

inline void rTrim(std::string& str)
{
    const auto it = sf::base::findIf(
        str.rbegin(), str.rend(), [](char ch) { return !std::isspace(ch); });

    str.erase(it.base(), str.end());
}

inline void lrTrim(std::string& str)
{
    lTrim(str);
    rTrim(str);
}

[[nodiscard]] inline std::string getLTrim(std::string s)
{
    lTrim(s);
    return s;
}

[[nodiscard]] inline std::string getRTrim(std::string s)
{
    rTrim(s);
    return s;
}

[[nodiscard]] inline std::string getLRTrim(std::string s)
{
    lrTrim(s);
    return s;
}

inline void uppercasify(std::string& s)
{
    for (auto& c : s)
    {
        c = std::toupper(c);
    }
}

[[nodiscard]] inline std::string toUppercase(std::string s)
{
    uppercasify(s);
    return s;
}

inline void lowercasify(std::string& s)
{
    for (auto& c : s)
    {
        c = std::tolower(c);
    }
}

[[nodiscard]] inline std::string toLowercase(std::string s)
{
    lowercasify(s);
    return s;
}

} // namespace hg::Utils

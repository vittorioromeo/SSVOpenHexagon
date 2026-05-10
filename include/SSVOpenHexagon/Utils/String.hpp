// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"

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

// Append `value` to `out` in the "minimal" floating-point format that
// the pre-VRSFML codebase produced via `ssvu::toStr`:
//
//   `0.5f → "0.5"`, `1.0f → "1"`, `1.6f → "1.6"`, `12.345f → "12.345"`.
//
// `sf::base::appendToString` always uses fixed 6-decimal precision
// (`"0.500000"`); we trim the trailing zeros and a dangling decimal
// point. Required by anything keyed off the old format -- score
// validators in `_RELEASE/config.json`, server `Database::Score::levelValidator`
// rows, replay file names + their internal `_level_id_validator`.
inline void appendMinimalFloat(sf::base::String& out, const float value)
{
    const sf::base::SizeT before = out.size();
    sf::base::appendToString(out, value);

    // Only trim the fractional tail. Integers (no '.') are already minimal.
    bool hasDot = false;
    for (sf::base::SizeT i = before; i < out.size(); ++i)
    {
        if (out[i] == '.')
        {
            hasDot = true;
            break;
        }
    }

    if (!hasDot)
    {
        return;
    }

    while (out.size() > before && out[out.size() - 1] == '0')
    {
        out.popBack();
    }

    if (out.size() > before && out[out.size() - 1] == '.')
    {
        out.popBack();
    }
}

[[nodiscard]] inline sf::base::String toMinimalFloatString(const float value)
{
    sf::base::String out;
    appendMinimalFloat(out, value);
    return out;
}

} // namespace hg::Utils

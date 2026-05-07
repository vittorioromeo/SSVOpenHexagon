// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LevelValidator.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToString.hpp"


namespace hg::Utils
{

namespace
{

// Append `value` to `out` in the *minimal* "shortest-round-trip"
// representation -- e.g. `0.5f → "0.5"`, `1.0f → "1"`, `1.6f → "1.6"`.
//
// Pre-VRSFML the codebase used `ssvu::toStr` which produced this
// minimal format. The migration replaced it with `appendToString`, which
// uses fixed 6-decimal precision (`0.500000`, `1.000000`, ...) and broke
// every external consumer that stored or compared validator strings:
//
//   - `_RELEASE/config.json` server_level_whitelist entries (`m_0.5`)
//   - `Database::Score::levelValidator` rows (server-side score keys)
//   - On-disk replay file names + their internal `_level_id_validator`
//
// All of those are keyed off the old minimal format, so we keep
// generating it here. Implementation: format with the standard 6-digit
// precision, then trim trailing zeros and a dangling decimal point.
void appendMinimalFloat(sf::base::String& out, const float value)
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

    // Pop trailing '0's, then the '.' if it ended up alone. Done via
    // `resize` since `sf::base::String` doesn't expose a single-char pop.
    sf::base::SizeT newEnd = out.size();

    while (newEnd > before && out[newEnd - 1] == '0')
    {
        --newEnd;
    }

    if (newEnd > before && out[newEnd - 1] == '.')
    {
        --newEnd;
    }

    out.resize(newEnd);
}

} // namespace

[[nodiscard]] sf::base::String getLevelValidator(const sf::base::StringView levelId, const float diffMult)
{
    sf::base::String result;

    result += levelId;
    result += "_m_";
    appendMinimalFloat(result, diffMult);

    return result;
}

} // namespace hg::Utils

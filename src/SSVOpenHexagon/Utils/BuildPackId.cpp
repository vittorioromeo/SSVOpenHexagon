// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/BuildPackId.hpp"

#include <SFML/Base/String.hpp>
#include <SFML/Base/StringView.hpp>
#include <SFML/Base/ToString.hpp>


namespace hg::Utils
{

[[nodiscard]] sf::base::String buildPackId(const sf::base::StringView packDisambiguator,
                                           const sf::base::StringView packAuthor,
                                           const sf::base::StringView packName,
                                           const int                  packVersion)
{
    SSVOH_ASSERT(!packDisambiguator.empty());
    SSVOH_ASSERT(!packAuthor.empty());
    SSVOH_ASSERT(!packName.empty());
    SSVOH_ASSERT(packVersion > 0);

    const auto spaceToUnderscore = [](const char c) { return (c == ' ' || c == '\n' || c == '\t') ? '_' : c; };

    sf::base::String result;

    for (const char c : packDisambiguator)
        result += spaceToUnderscore(c);
    result += '_';
    for (const char c : packAuthor)
        result += spaceToUnderscore(c);
    result += '_';
    for (const char c : packName)
        result += spaceToUnderscore(c);
    result += '_';
    sf::base::appendToString(result, packVersion);

    return result;
}

} // namespace hg::Utils

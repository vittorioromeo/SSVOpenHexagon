// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LevelValidator.hpp"

#include <SFML/Base/String.hpp>
#include <SFML/Base/StringView.hpp>
#include <SFML/Base/ToString.hpp>


namespace hg::Utils
{

[[nodiscard]] sf::base::String getLevelValidator(const sf::base::StringView levelId, const float diffMult)
{
    sf::base::String result;

    result += levelId;
    result += "_m_";
    sf::base::appendToString(result, diffMult);

    return result;
}

} // namespace hg::Utils

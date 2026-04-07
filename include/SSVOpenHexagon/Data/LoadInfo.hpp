// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


namespace hg
{

struct LoadInfo
{
    unsigned int                       packs{0};
    unsigned int                       levels{0};
    unsigned int                       assets{0};
    sf::base::Vector<sf::base::String> errorMessages;

    void addFormattedError(sf::base::String& error);
};

} // namespace hg

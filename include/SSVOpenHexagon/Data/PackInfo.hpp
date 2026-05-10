// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/System/Path.hpp"

#include "SFML/Base/String.hpp"


namespace hg
{

struct PackInfo
{
    sf::base::String id;
    sf::Path         path;
};

} // namespace hg

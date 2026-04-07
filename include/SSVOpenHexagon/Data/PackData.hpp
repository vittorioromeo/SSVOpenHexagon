// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


namespace hg
{

struct PackDependency
{
    sf::base::String disambiguator;
    sf::base::String name;
    sf::base::String author;
    int              minVersion;
};

struct PackData
{
    sf::base::String                 folderPath;
    sf::base::String                 id;
    sf::base::String                 disambiguator;
    sf::base::String                 name;
    sf::base::String                 author;
    sf::base::String                 description;
    int                              version;
    float                            priority;
    sf::base::Vector<PackDependency> dependencies;
};

} // namespace hg

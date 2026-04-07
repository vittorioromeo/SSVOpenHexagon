// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/String.hpp"

namespace sf::base
{
class String;
class StringView;
} // namespace sf::base

namespace hg::Utils
{

[[nodiscard]] sf::base::String buildPackId(const sf::base::StringView packDisambiguator,
                                           const sf::base::StringView packAuthor,
                                           const sf::base::StringView packName,
                                           const int                  packVersion);

} // namespace hg::Utils

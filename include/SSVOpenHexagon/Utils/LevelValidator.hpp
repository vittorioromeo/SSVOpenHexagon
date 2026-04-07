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

[[nodiscard]] sf::base::String getLevelValidator(const sf::base::StringView levelId, const float diffMult);

} // namespace hg::Utils

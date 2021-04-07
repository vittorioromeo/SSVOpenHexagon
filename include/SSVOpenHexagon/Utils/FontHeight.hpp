// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVStart/Utils/SFML.hpp>

#include <SFML/Graphics/Text.hpp>

namespace hg::Utils
{

[[nodiscard]] inline float getFontHeight(sf::Text& font)
{
    font.setString("ABCDEFGHILMNOPQRSTUVZ:");
    return ssvs::getGlobalHeight(font);
}

[[nodiscard]] inline float getFontHeight(
    sf::Text& font, const unsigned int charSize)
{
    font.setCharacterSize(charSize);
    return getFontHeight(font);
}

} // namespace hg::Utils

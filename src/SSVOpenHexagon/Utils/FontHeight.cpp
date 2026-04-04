// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/FontHeight.hpp"

#include "SFML/Graphics/Text.hpp"

namespace hg::Utils
{

[[nodiscard]] float getFontHeight(sf::Text& font)
{
    font.setString("ABCDEFGHILMNOPQRSTUVZ:");
    return font.getGlobalBounds().size.y;
}

[[nodiscard]] float getFontHeight(sf::Text& font, const unsigned int charSize)
{
    const sf::Vec2f previousScale = font.scale;
    const float     baseSize      = static_cast<float>(font.getCharacterSize());
    const float     scale         = static_cast<float>(charSize) / baseSize;

    font.scale         = {scale, scale};
    const float result = getFontHeight(font);
    font.scale         = previousScale;

    return result;
}

} // namespace hg::Utils

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace sf {

class Text;

}

namespace hg::Utils {

[[nodiscard]] float getFontHeight(sf::Text& font);
[[nodiscard]] float getFontHeight(sf::Text& font, const unsigned int charSize);

} // namespace hg::Utils

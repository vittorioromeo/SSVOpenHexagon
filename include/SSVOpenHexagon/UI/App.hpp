// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// The full state of the new menu UI. One instance per running game.
// Each screen has a small named sub-struct here — replaces the ~80 ad-hoc
// member variables of the old `MenuGame` class.

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg::ui
{

enum class Screen : sf::base::U8
{
    Main = 0,
    // Phase 1+ will add: Options, Profile.
    // Phase 2+: LevelSelect.
    // Phase 3+: WorkshopBrowse.
    // Phase 4+: Online.
};

struct MainScreenState
{
    int   selectedIdx{0};
    float selectionY{0.f}; //!< animated selection cursor; lerps toward selectedIdx*rowHeight
    float openProgress{0.f}; //!< 0 -> 1 staggered fade-in on screen entry
};

struct App
{
    Screen current{Screen::Main};

    // Stack of screens to pop on `escape`.
    sf::base::Vector<Screen> backStack{};

    MainScreenState main{};

    // Tells the host (MenuGame) the new UI wants the game to quit.
    bool exitRequested{false};
};

} // namespace hg::ui

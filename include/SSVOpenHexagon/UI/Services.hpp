// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Callback bundle the new UI uses to talk to the rest of the game without
// depending on `MenuGame.hpp` (or anything else heavy). Each screen's draw
// function reads the App state and, when the user activates an action, calls
// one of these. The host (currently `MenuGame`) installs the callbacks at
// construction time, capturing whatever state they need.
//
// Keeping screens dependency-free of `MenuGame` is what lets us delete
// `MenuGame.cpp` cleanly at the end of the rewrite.

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/StringView.hpp"

namespace hg::ui
{

struct Services
{
    sf::base::FixedFunction<void(), 64> onExit;
    sf::base::FixedFunction<void(), 64> onPlayRequested;
    sf::base::FixedFunction<void(), 64> onOptionsRequested;
    sf::base::FixedFunction<void(), 64> onOnlineRequested;
    sf::base::FixedFunction<void(), 64> onProfileRequested;
    sf::base::FixedFunction<void(), 64> onWorkshopRequested;

    // Optional UX hook for sound feedback on selection moves; can be empty.
    sf::base::FixedFunction<void(sf::base::StringView), 64> playSound;
};

} // namespace hg::ui

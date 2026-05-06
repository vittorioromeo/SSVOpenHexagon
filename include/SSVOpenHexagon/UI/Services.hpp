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

namespace sf::base
{
class String;
} // namespace sf::base

namespace hg
{
class HGAssets;
class ProfileData;

namespace Steam
{
class steam_manager;
} // namespace Steam
} // namespace hg

namespace hg::ui
{

struct Services
{
    sf::base::FixedFunction<void(), 64> onExit;
    sf::base::FixedFunction<void(), 64> onPlayRequested;     //!< user clicked PLAY on main menu (legacy: opens old level select)
    sf::base::FixedFunction<void(), 64> onOptionsRequested;
    sf::base::FixedFunction<void(), 64> onOnlineRequested;
    sf::base::FixedFunction<void(), 64> onProfileRequested;
    sf::base::FixedFunction<void(), 64> onWorkshopRequested;

    // Start a level. `levelId` is the LevelData id; `difficultyMult` picks
    // which difficulty index. Called by the new Level Select screen.
    sf::base::FixedFunction<void(const sf::base::String& /*levelId*/,
                                  float /*difficultyMult*/),
                              128>
        onStartLevel;

    // Optional UX hook for sound feedback on selection moves; can be empty.
    sf::base::FixedFunction<void(sf::base::StringView), 64> playSound;

    // Direct pointers to backend systems screens read from. Phase 2's Level
    // Select reads pack/level metadata via these; Phase 3's Workshop browser
    // calls Steam UGC and `installPackAtRuntime` directly. Any of these may
    // be null in non-game contexts (e.g. unit tests).
    HGAssets*             assets{nullptr};
    ProfileData*          currentProfile{nullptr};
    Steam::steam_manager* steamManager{nullptr};
};

} // namespace hg::ui

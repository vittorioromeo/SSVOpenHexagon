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

namespace sf
{
class RenderTexture;
} // namespace sf

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

    // Online-screen actions. Each delegates to the legacy `HexagonClient`
    // and dialog flow; the buttons in the new Online screen call the ones
    // applicable to the current state (driven by `profileSnapshot.onlineStatus`).
    sf::base::FixedFunction<void(), 64> onOnlineConnect;
    sf::base::FixedFunction<void(), 64> onOnlineDisconnect;
    sf::base::FixedFunction<void(), 64> onOnlineLogin;
    sf::base::FixedFunction<void(), 64> onOnlineLogout;
    sf::base::FixedFunction<void(), 64> onOnlineRegister;

    // Start a level. `levelId` is the LevelData id; `difficultyMult` picks
    // which difficulty index. Called by the new Level Select screen.
    sf::base::FixedFunction<void(const sf::base::String& /*levelId*/,
                                  float /*difficultyMult*/),
                              128>
        onStartLevel;

    // Notify the host that the LevelSelect cursor has moved to a different
    // level. Lets the legacy backdrop refresh its style/colors/preview to
    // match. `levelId` is the pack-prefixed asset key. Called at most once
    // per change of `selectedIdx`.
    sf::base::FixedFunction<void(const sf::base::String& /*levelId*/), 64>
        onPreviewLevel;

    // Optional UX hook for sound feedback on selection moves; can be empty.
    sf::base::FixedFunction<void(sf::base::StringView), 64> playSound;

    // Direct pointers to backend systems screens read from. Phase 2's Level
    // Select reads pack/level metadata via these; Phase 3's Workshop browser
    // calls Steam UGC and `installPackAtRuntime` directly. Any of these may
    // be null in non-game contexts (e.g. unit tests).
    HGAssets*             assets{nullptr};
    ProfileData*          currentProfile{nullptr};
    Steam::steam_manager* steamManager{nullptr};

    // Off-screen render target the host paints with the currently-selected
    // level's running visuals. The LevelSelect screen samples it as a
    // sprite to display a live preview of the level. Null when no preview
    // is available (no level loaded yet, or running headless).
    sf::RenderTexture*    previewTexture{nullptr};
};

} // namespace hg::ui

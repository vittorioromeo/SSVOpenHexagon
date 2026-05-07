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
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/StringView.hpp"

#include <unordered_set>

namespace sf
{
class RenderTarget;
class RenderTexture;
} // namespace sf

namespace sf::base
{
class String;

template <typename>
class Vector;
} // namespace sf::base

namespace hg
{
class HGAssets;
class ProfileData;

namespace Database
{
struct ProcessedScore;
} // namespace Database

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

    // Render target screens should use to bypass the menu's accent-gradient
    // shader pass. Points at the window when the host is compositing the
    // UI through `menuAccentGradient.frag`; null otherwise (headless / no
    // composite texture). Used by the LevelSelect preview, whose magenta
    // pixels would otherwise be remapped by the shader. Drawing into this
    // target uses the same view + transform as `ctx.renderStates`, so
    // positions match the rest of the UI.
    sf::RenderTarget*     rawTarget{nullptr};

    // Notify the host that the LevelSelect cursor / difficulty changed.
    // The host computes the level validator, requests fresh top scores
    // from `HexagonClient` (rate-limited via `LeaderboardCache`), and
    // surfaces the latest cached snapshot via `leaderboardScores` below.
    // Called every frame the LevelSelect column is visible -- the host
    // de-duplicates by validator, so polling is cheap.
    sf::base::FixedFunction<void(const sf::base::String& /*levelId*/,
                                  float /*difficultyMult*/),
                              64>
        onRequestLeaderboard;

    // Request the server-stored replay for the leaderboard row at
    // `scoreTimestamp`. The host already knows which (level, difficulty)
    // is "current" because `onRequestLeaderboard` updates it every frame
    // -- so we don't bother re-passing those args. The host fires
    // `HexagonClient::tryRequestReplay`; when the reply arrives,
    // `MenuGame` swaps the foreground gameplay HG into replay playback.
    // No-op if the user isn't online or the replay isn't on the server's
    // disk.
    sf::base::FixedFunction<void(sf::base::U64 /*scoreTimestamp*/), 64> onWatchReplay;

    // Latest cached top scores for whatever (level, difficulty) was last
    // requested via `onRequestLeaderboard`. Null until data has arrived.
    // Pointer stability is provided by `LeaderboardCache` (entries are
    // never erased), so the screen can read this between frames safely.
    const sf::base::Vector<Database::ProcessedScore>* leaderboardScores{nullptr};

    // Per-(level, difficulty) set of score timestamps the server told us
    // have no replay on disk. Drives the inline "(NO REPLAY)" markers
    // next to leaderboard rows. Always non-null -- when no info exists
    // the host points it at a static empty set so the screen can do
    // unconditional `contains` lookups.
    const std::unordered_set<sf::base::U64>* leaderboardUnavailable{nullptr};

    // Where the leaderboard pipeline currently is. Drives the empty-state
    // message in the LevelSelect leaderboard column. Distinguishing
    // these states matters for diagnostics: "LOADING" used to be the
    // catch-all and would mask hard failures (e.g. a request rejected
    // because the client never reached `LoggedIn_Ready`).
    enum class LeaderboardStatus : sf::base::U8
    {
        Offline = 0, //!< no TCP connection / Steam ticket / login token yet
        Connecting,  //!< handshake in progress; can't issue requests yet
        Loading,     //!< request sent to server, awaiting `EReceivedTopScores`
        Ready,       //!< server has replied; `leaderboardScores` is populated (possibly empty)
        Unsupported, //!< server doesn't have this validator on its whitelist
    };
    LeaderboardStatus leaderboardStatus{LeaderboardStatus::Offline};
};

} // namespace hg::ui

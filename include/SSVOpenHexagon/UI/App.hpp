// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// The full state of the new menu UI. One instance per running game.
// Each screen has a small named sub-struct here -- replaces the ~80 ad-hoc
// member variables of the old `MenuGame` class.

#include "SSVOpenHexagon/Core/Steam.hpp" // for `WorkshopItem`

#include "SFML/Graphics/Texture.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg::ui
{

enum class Screen : sf::base::U8
{
    Main = 0,
    Options,
    LevelSelect,
    WorkshopBrowse,
    Online,
};

struct MainScreenState
{
    int   selectedIdx{0};
    float selectionY{0.f};   //!< animated selection cursor (lerps toward selectedIdx*rowHeight)
    float openProgress{0.f}; //!< 0 -> 1 staggered fade-in on screen entry (only Main uses this)
};

struct OnlineScreenState
{
    int   selectedIdx{0};
    float selectionY{0.f}; //!< animated y-pixel cursor on the action list
};

struct OptionsScreenState
{
    // Two-pane layout: a category list on the left, items in the selected
    // category on the right. Index `0` of `selectedItem` corresponds to the
    // first item in the currently-selected category; `-1` means focus is on
    // the category list.
    int   selectedCategory{0};
    int   selectedItem{-1};
    float categorySelectionY{0.f};
    float itemSelectionY{0.f};

    // True while the focused slider is in "edit mode" -- left/right then
    // step the slider's value instead of navigating panes. Toggled on by
    // Enter on a slider row, off by Enter again, by Escape, or by
    // up/down (which also moves the focus away).
    bool sliderEditing{false};
};

enum class LevelSortKey : sf::base::U8
{
    PackPriority = 0, //!< default -- pack-priority then menu-priority
    Name,
    Author,
    MinDifficulty,
    PersonalBest,
    LastPlayed,
    PlayCount,
};

struct WorkshopBrowseScreenState
{
    Steam::WorkshopQueryMode              queryMode{Steam::WorkshopQueryMode::MostPopular};
    int                                   page{1};
    sf::base::Vector<Steam::WorkshopItem> items;

    bool queryInFlight{false};
    bool initialQueryFired{false};

    // Total result count Steam reports for the active query -- used to
    // render the page indicator as "current/total". 0 until the first
    // query lands.
    sf::base::U32 totalMatching{0};

    // Client-side text filter applied to the items received from Steam.
    // Filters by title + description. Empty = show all.
    sf::base::String search;

    // Resolved {publishedFileId, title} pairs used to display dependency
    // names. Populated from every QueryComplete event (regular + details).
    // Linear scan -- count is small (≤ a handful per item).
    struct WorkshopNameEntry
    {
        sf::base::U64    publishedFileId{};
        sf::base::String title;
    };
    sf::base::Vector<WorkshopNameEntry> nameCache;

    // IDs we've already requested a details query for, to avoid spamming
    // Steam every frame. Reset on screen exit.
    sf::base::Vector<sf::base::U64> requestedNameLookups;

    int   selectedIdx{0};
    float selectionY{0.f};

    // Three-pane focus: 0 = sidebar (filters / pagination), 1 = list,
    // 2 = actions. Left/right arrows hop between adjacent panes via the
    // shared `paneSwitchLeftRight` helper; up/down navigate the active
    // pane. Default is the list, since that's where the user's attention
    // naturally goes after entering the screen.
    int activePane{1};

    int   sidebarIdx{0};
    float sidebarSelectionY{0.f};

    // Show only items that are subscribed AND installed. Useful for
    // re-finding packs the user has already pulled down.
    bool downloadedOnly{false};

    // Action-pane row index (DOWNLOAD/DELETE = 0, BACK = 1).
    int   actionIdx{0};
    float actionSelectionY{0.f};

    // Async preview-image cache, keyed by `publishedFileId`. The flow:
    //   1. UI navigates to an item; if not in the map, it fires
    //      `request_workshop_preview` and inserts the id with empty
    //      optional (= "in flight, no texture yet").
    //   2. `PreviewDownloaded` event arrives in the host's event pump,
    //      which decodes the bytes into a texture and stores it back
    //      into this map.
    //   3. The render path looks the id up; if the optional has a
    //      texture it's drawn, otherwise the placeholder shows.
    // Persisting across screen visits avoids re-downloading every time
    // the user returns to the workshop browser.
    ankerl::unordered_dense::map<sf::base::U64, sf::base::Optional<sf::Texture>> previewTextures;

    char statusMessage[128] = {};
};

struct LevelSelectScreenState
{
    // Search / filter / sort. `filteredLevelIds` is recomputed (in the
    // screen's draw function) whenever any of these change OR when the
    // assets' `packListVersion()` advances.
    sf::base::String search;
    LevelSortKey     sortKey{LevelSortKey::PackPriority};
    bool             favoritesOnly{false};

    // Cached filter/sort result, owned here. `cachedPackListVersion` is
    // compared against `assets.packListVersion()` to invalidate.
    // `filteredDisplayRows[i]` is the display-row index of the i-th level
    // (accounting for inserted pack header rows when grouping is active).
    sf::base::Vector<sf::base::String> filteredLevelIds;
    sf::base::Vector<int>              filteredDisplayRows;
    sf::base::U64                      cachedPackListVersion{0};
    bool                               cacheStale{true};

    // Selection within `filteredLevelIds`.
    int              levelIdx{0};
    int              difficultyIdx{0};
    sf::base::String lastPreviewedId{};   //!< last level *id* we fired `onPreviewLevel` for
    int              scrollStart{0};      //!< persistent first-visible index for the level list
    int              packScrollStart{0};  //!< persistent first-visible index for the pack column
    float            selectionY{0.f};     //!< animated y-pixel cursor on the level list (visible space)
    float            packSelectionY{0.f}; //!< animated y-pixel cursor on the pack column

    // Which column gets keyboard up/down/enter input. Left/right arrows
    // cycle between panes; clicking on a pack/level/action also moves the
    // focus there. The unfocused column shows a desaturated pill.
    enum class Pane : sf::base::U8
    {
        Packs = 0,
        Levels,
        Actions,
        Leaderboard, //!< online top-scores column on the far right
    };
    Pane pane{Pane::Levels};

    // Index inside the actions pane (FAVORITE, PLAY, DIFFICULTY, BACK).
    int   actionIdx{1}; //!< default to PLAY
    float actionSelectionY{0.f};

    // Selection inside the leaderboard pane. Index into
    // `Services::leaderboardScores` once it has data; otherwise the
    // pane renders empty-state messages and the index is just held for
    // continuity when scores arrive.
    int   leaderboardIdx{0};
    float leaderboardSelectionY{0.f};

    // 0..1 alpha for the live level-preview pane on the right. Eases in
    // when a preview becomes available, eases out when it's torn down,
    // so navigating between preview-less and preview-bearing states no
    // longer pops the texture in instantly.
    float previewAlpha{0.f};
};

// Read-only summary the host (MenuGame) refreshes each frame before calling
// `drawCurrentScreen`. Lets the screens render real data without depending
// on `HGAssets`/`HexagonClient`/etc.
struct ProfileSnapshot
{
    char name[64]         = {};
    int  totalScored      = 0;
    int  totalFavorites   = 0;
    char onlineStatus[64] = "OFFLINE";

    // Capability flags driven by the host from `HexagonClient::State`. The
    // Online screen reads these to decide which action buttons to surface
    // -- avoids reverse-engineering the state from `onlineStatus`.
    bool canConnect{false};
    bool canDisconnect{false};
    bool canLogIn{false};
    bool canLogOut{false};
    bool canRegister{false};
};

struct App
{
    Screen current{Screen::Main};

    // What the user was looking at before the latest push/pop. Lets the
    // dispatcher keep rendering the outgoing sub-screen while it fades
    // out during a pop (when `current` has already become Main).
    Screen previousScreen{Screen::Main};

    // Stack of screens to pop on `escape`.
    sf::base::Vector<Screen> backStack{};

    // Animates 0 → 1 as soon as a sub-screen replaces the Main menu,
    // back to 0 when the user pops back to Main. The dispatcher uses it
    // to slide + fade the Main menu into a partially-OOB backdrop, so
    // sub-screens feel "stacked" on top of Main rather than replacing it.
    // Time-based linear advance (fixed-duration); easing applied at render
    // time so push and pop share a single advance loop.
    float backDepthAnim{0.f};

    MainScreenState           main{};
    OptionsScreenState        options{};
    LevelSelectScreenState    levelSelect{};
    WorkshopBrowseScreenState workshop{};
    OnlineScreenState         online{};

    // Snapshots refreshed by the host each frame.
    ProfileSnapshot profileSnapshot{};

    // Tells the host (MenuGame) the new UI wants the game to quit.
    bool exitRequested{false};
};

} // namespace hg::ui

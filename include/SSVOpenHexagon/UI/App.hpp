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
    Profile,
    Options,
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

struct ProfileScreenState
{
    int   selectedIdx{0};
    float selectionY{0.f};
    float openProgress{0.f};
};

struct OptionsScreenState
{
    // Two-pane layout: a category list on the left, items in the selected
    // category on the right. Index `0` of `selectedItem` corresponds to the
    // first item in the currently-selected category.
    int   selectedCategory{0};
    int   selectedItem{-1};   //!< -1 = focus is on the category list
    float openProgress{0.f};
    float categorySelectionY{0.f};
    float itemSelectionY{0.f};
};

// Read-only summary the host (MenuGame) refreshes each frame before calling
// `drawCurrentScreen`. Lets the screens render real data without depending
// on `HGAssets`/`HexagonClient`/etc.
struct ProfileSnapshot
{
    char name[64]            = {};
    int  totalScored         = 0;
    int  totalFavorites      = 0;
    char onlineStatus[64]    = "OFFLINE";
};

struct App
{
    Screen current{Screen::Main};

    // Stack of screens to pop on `escape`.
    sf::base::Vector<Screen> backStack{};

    MainScreenState    main{};
    ProfileScreenState profile{};
    OptionsScreenState options{};

    // Snapshots refreshed by the host each frame.
    ProfileSnapshot profileSnapshot{};

    // Tells the host (MenuGame) the new UI wants the game to quit.
    bool exitRequested{false};
};

} // namespace hg::ui

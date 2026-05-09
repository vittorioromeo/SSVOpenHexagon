# UI rewrite -- design

Status: **proposal**, not yet implemented.
Companion to: [UI_REWRITE_RESEARCH.md](UI_REWRITE_RESEARCH.md). Read that first; this
document only references findings, it doesn't repeat them.
Style: KISS / YAGNI. Free functions, POD data, immediate drawing, no
retained geometry, no caching unless we measure a need for it.

---

## 1. Principles

These are non-negotiable. They drive every decision below.

1. **Immediate-mode rendering.** Every widget calls `window.draw(...)`
   directly with a fresh `sf::TextData{...}` / `sf::RectangleShape{...}`
   each frame. No vertex caches, no retained geometry, no dirty bits.
   If profiling later shows a real cost, we revisit the specific hot
   spot -- not the architecture.
2. **No widget hierarchy.** Widgets are free functions returning
   `bool` or a value. No `class Widget`, no virtual dispatch, no
   factory, no registry. The current `Menu`/`Category`/`ItemBase`
   tree disappears -- replaced by, at most, a `std::vector<MenuItem>`
   where `MenuItem` is a small tagged struct.
3. **Each screen is one function.** `void drawMainMenu(UI&, App&)`.
   No state machine framework, no transition table. The screen the
   game shows is decided by a single `App::Screen` enum and dispatched
   in one switch.
4. **App owns all state.** A single `App` struct (with named, typed
   sub-structs per screen) replaces the ~80 ad-hoc members of
   `MenuGame`. Widgets never own state -- they're given a reference to
   the field they edit.
5. **No layout solver.** Layout is "draw the next thing below the
   previous thing, indented by N." A `cursor: sf::Vector2f` on the
   `UI` context is enough. Two-column layouts use two cursors.
6. **No focus framework.** Each screen handles its own arrow-key
   navigation by maintaining a `selectedIndex` int. The widget gets
   passed `focused = (i == selectedIndex)` and renders accordingly.
   Mouse hover sets the same int. That's it.
7. **One UI source file per screen.** When `drawMainMenu()` grows
   past a few hundred lines, it gets its own .cpp. Never one big file
   like `MenuGame.cpp` again.
8. **Coexists with the old UI during transition.** New screens slot
   into the existing `States` enum first, then we delete the old
   path once everything is migrated. Section 8 describes phasing.

---

## 2. The UI primitive set

**One** header, **one** translation unit, fewer than ~500 lines total
across both. Lives at:

- `include/SSVOpenHexagon/UI/UI.hpp`
- `src/SSVOpenHexagon/UI/UI.cpp`

### 2.1 Context

```cpp
namespace ohui
{

// Per-frame snapshot of the input state the UI cares about. Filled
// once at the top of the menu's update tick -- no SFML event polling
// inside widgets.
struct Input
{
    sf::Vector2f mousePos;
    bool         mousePressed;   // edge: just pressed this frame
    bool         mouseDown;      // level: held this frame
    bool         up, down, left, right;     // edge
    bool         enter, escape, backspace;  // edge
    sf::base::StringView typedChars;        // for text fields
};

// Drawing + layout state. Lives on the menu controller, passed by
// reference to every widget. POD. No allocations.
struct Context
{
    sf::RenderTarget& target;
    const sf::Font&   font;
    Input             input;

    // Layout cursor. Widgets advance it as they draw.
    sf::Vector2f cursor;
    sf::Vector2f origin;   // where `newColumn()` resets to
    float        rowHeight = 28.f;

    // Theme -- fields, not virtual lookups.
    sf::Color colText      { 220, 220, 220 };
    sf::Color colTextDim   { 130, 130, 130 };
    sf::Color colHighlight { 255, 255, 255 };
    sf::Color colAccent    {  50, 150, 255 };
    float     fontSize     = 22.f;
};

} // namespace ohui
```

### 2.2 Layout helpers

Exactly the helpers that turn out to be needed; no more.

```cpp
void newLine(Context&, float dyExtra = 0.f);   // advance cursor.y by rowHeight + dyExtra
void newColumn(Context&, float dx);            // cursor.y = origin.y, cursor.x += dx
void indent(Context&, float dx);               // cursor.x += dx
sf::FloatRect measureRow(const Context&, float w); // rect at cursor with current rowHeight
```

### 2.3 Widgets (free functions)

These are the **only** primitives. Anything else is built ad hoc by
combining them.

```cpp
// Display only.
void label    (Context&, sf::base::StringView text);
void heading  (Context&, sf::base::StringView text);   // larger, draws separator below
void separator(Context&);

// Returns true the frame the user activates (clicks or Enter while focused).
bool button   (Context&, sf::base::StringView text, bool focused);

// Returns true if `value` was changed this frame.
bool toggle   (Context&, sf::base::StringView text, bool& value, bool focused);
bool slider   (Context&, sf::base::StringView text, float& v, float min, float max, bool focused);

// One-line text input. Returns true if `out` changed; on Enter sets
// `submitted` to true.
bool textField(Context&, sf::base::StringView label, sf::base::String& out,
               bool focused, bool& submitted);

// Returns true on click anywhere in `rect`.
bool clickArea(Context&, sf::FloatRect rect);

// Generic "rectangle with a label inside it" -- used to build custom
// widgets like level-list rows without proliferating helper types.
struct RowResult { bool clicked; bool hovered; };
RowResult row    (Context&, sf::FloatRect rect, sf::base::StringView text, bool focused);
```

### 2.4 What's deliberately not provided

- No window/panel/group containers. Screens position things directly.
- No tab/treeview/combobox. If we need a combobox, it's a `button` that
  pops a small list -- written ad hoc when the first need arises.
- No animation framework. If a screen wants animation, it stores the
  float on its sub-struct and lerps it manually in its draw function.
- No theme stack/push-pop. Theme is the `Context` field; mutate it
  directly if a screen wants different colours, restore on exit.
- No "automatic ID system." Focus is per-screen index, not framework-
  managed.

---

## 3. Screen dispatcher

Replaces the `States` enum + `getCurrentMenu()` + every `*Action()`
cascade with a single switch.

```cpp
enum class Screen : sf::base::U8
{
    Loading,
    Epilepsy,
    Main,
    Profile,           // collapsed: just "offline" entry + login
    Options,
    LevelSelect,
    WorkshopBrowse,    // new
    Online,            // login + recovery
};

struct App
{
    Screen current = Screen::Loading;

    // Screen-specific sub-structs (each one ~10-30 named fields).
    struct MainScreen      { int selectedIdx = 0; } main;
    struct OptionsScreen   { int selectedIdx = 0; int section = 0; } options;
    struct LevelSelectScreen
    {
        int                          packIdx       = 0;
        int                          levelIdx      = 0;
        int                          difficultyIdx = 0;
        sf::base::String             searchText;
        SortKey                      sortKey       = SortKey::PackPriority;
        FilterSpec                   filter;
        bool                         showFavoritesOnly = false;
        sf::base::Vector<sf::base::String> filteredLevelIds; // recomputed when search/sort/filter change
    } levelSelect;
    struct WorkshopScreen  { /* ... */ } workshop;
    struct OnlineScreen    { /* login form, recovery flow */ } online;
    // ...

    // Cross-screen state.
    sf::base::Vector<Screen> backStack; // for `goBack()` semantics
};

void drawCurrentScreen(ohui::Context& ui, App& app, /* refs to assets, client, ... */)
{
    switch (app.current)
    {
        case Screen::Main:           drawMainMenu(ui, app); break;
        case Screen::Options:        drawOptions(ui, app); break;
        case Screen::LevelSelect:    drawLevelSelect(ui, app, ...); break;
        case Screen::WorkshopBrowse: drawWorkshopBrowse(ui, app, ...); break;
        case Screen::Online:         drawOnline(ui, app, ...); break;
        case Screen::Profile:        drawProfile(ui, app); break;
        case Screen::Loading:        drawLoading(ui, app); break;
        case Screen::Epilepsy:       drawEpilepsy(ui, app); break;
    }
}
```

Navigation: `app.current = X;` for forward, `app.current = app.backStack.back(); app.backStack.pop_back();` for back. No `Goto`/`GoBack` item types; they were just data.

---

## 4. Sketch of one screen end-to-end

To make the style concrete, here's `drawMainMenu` (10 named menu
items today; would be ~30 lines instead of the current
73-line `drawMainMenu()` at
[MenuGame.cpp:3653](../src/SSVOpenHexagon/Core/MenuGame.cpp#L3653)):

```cpp
void drawMainMenu(ohui::Context& ui, App& app)
{
    static constexpr const char* items[] = {
        "Play", "Workshop", "Options", "Profile", "Online", "Exit"
    };
    constexpr int N = std::ssize(items);

    if (ui.input.up)   app.main.selectedIdx = (app.main.selectedIdx + N - 1) % N;
    if (ui.input.down) app.main.selectedIdx = (app.main.selectedIdx + 1) % N;

    ui.cursor = ui.origin = { ui.target.getSize().x * 0.5f - 200.f, 250.f };
    heading(ui, "OPEN HEXAGON");
    newLine(ui, 16.f);

    for (int i = 0; i < N; ++i)
    {
        if (ohui::button(ui, items[i], i == app.main.selectedIdx))
        {
            app.main.selectedIdx = i;
            switch (i)
            {
                case 0: app.current = Screen::LevelSelect;    break;
                case 1: app.current = Screen::WorkshopBrowse; break;
                case 2: app.current = Screen::Options;        break;
                case 3: app.current = Screen::Profile;        break;
                case 4: app.current = Screen::Online;         break;
                case 5: app.shouldQuit = true;                break;
            }
            return;
        }
        newLine(ui);
    }
}
```

Compare to today's flow:
[MenuGame.hpp's mainMenu Menu object](../include/SSVOpenHexagon/Core/MenuGame.hpp)
+ [setup code building the Category/ItemBase tree](../src/SSVOpenHexagon/Core/MenuGame.cpp)
+ [drawMainMenu rendering](../src/SSVOpenHexagon/Core/MenuGame.cpp#L3653)
+ [upAction/downAction/okAction cascades](../src/SSVOpenHexagon/Core/MenuGame.cpp#L1909-L2437)
across hundreds of lines, four files, and a lot of indirection.

---

## 5. Backend changes mapped to flaws

These follow the [research document's catalogue](UI_REWRITE_RESEARCH.md#5-catalogue-of-flaws--root-causes)
1-to-1. Each is the smallest viable change.

### 5.1 F1 -- Workshop in-game (Steam API expansion)

Add to
[`Steam::steam_manager`](../include/SSVOpenHexagon/Core/Steam.hpp):

```cpp
// Async query -- wraps SteamUGC()->CreateQueryAllUGCRequest. Result is
// delivered via the event queue (below). Returns a `QueryHandle`
// the caller can match against the result event.
struct WorkshopItem
{
    sf::base::U64    publishedFileId;
    sf::base::String title;
    sf::base::String description;
    sf::base::String previewImageUrl;
    sf::base::U64    sizeBytes;
    bool             isSubscribed;
    bool             isInstalled;
};

QueryHandle queryWorkshopItems(int page, /* sort / filter args */);

void subscribeWorkshopItem  (sf::base::U64 publishedFileId);
void unsubscribeWorkshopItem(sf::base::U64 publishedFileId);

// Polling event channel. `Event` is a tagged union over a small set:
//   QueryComplete { handle, items }
//   ItemInstalled { publishedFileId, folderPath }
//   ItemSubscribed   / Unsubscribed
//   DownloadProgress { publishedFileId, bytesDone, bytesTotal }
sf::base::Optional<Event> pollWorkshopEvent();
```

Wires up `SteamAPICall_t` for `SteamUGC::SubscribeItem` etc., plus
the missing callbacks (`ItemInstalled_t`,
`SubscribedItemsListChanged_t`, `DownloadItemResult_t`) that the
research [4.1](UI_REWRITE_RESEARCH.md#41-steam-workshop-integration--read-only-and-one-shot)
flagged as not registered.

The UI's workshop browse screen calls `queryWorkshopItems` once on
entry, draws a paginated list from the result, and on the user
clicking "Subscribe" calls `subscribeWorkshopItem`. The
download/install happens asynchronously; when the
`ItemInstalled` event arrives, the UI invokes the asset hot-install
path (next bullet).

### 5.2 F2 -- Hot install (asset reload)

Add to [`HGAssets`](../include/SSVOpenHexagon/Global/Assets.hpp):

```cpp
// Returns true on success. Idempotent (re-installing an existing pack
// just refreshes its data, like the dev-time reloadPack does today).
[[nodiscard]] bool installPackAtRuntime(const sf::base::String& folderPath);

[[nodiscard]] bool uninstallPack(const sf::base::String& packId);

// Snapshot version counter -- bumped whenever the pack/level lists
// change. The UI compares against its cached version each frame and
// rebuilds its filtered view when they differ. No callbacks needed.
sf::base::U64 packListVersion() const noexcept;
```

`installPackAtRuntime` factors out the per-pack work already in
[`loadPackAssets`](../src/SSVOpenHexagon/Global/Assets.cpp#L464-L527)
plus the dependency check from
[Assets.cpp:415-442](../src/SSVOpenHexagon/Global/Assets.cpp#L415-L442).
Synchronous on the main thread -- for a typical pack (a few levels +
a handful of music tracks + Lua scripts) it should be tens of
milliseconds, fine to stall a single frame in the menu. We don't
build a worker thread until we measure the actual stall.

The `packListVersion()` counter lets the level-select screen
re-derive its filtered/sorted list without ad-hoc callback wiring.

### 5.3 F3 -- Password recovery (Steam-ticket-based)

The simplest recovery channel is the one we already trust: **Steam
itself**. The user is logged into Steam; their Steam ID is the
primary key on `Database::User`; therefore "I forgot my password" can
be authenticated by re-doing the Steam encrypted-app-ticket exchange
that registration already uses
([HexagonClient.cpp:initializeTicketSteamID](../src/SSVOpenHexagon/Core/HexagonClient.cpp)).
No email, no SMTP, no recovery tokens.

Schema migration ([DatabaseRecords.hpp:User](../include/SSVOpenHexagon/Online/DatabaseRecords.hpp#L15-L21)):

```diff
 struct User
 {
     U32         id;
     U64         steamId;
     std::string name;
+    std::vector<char> passwordSalt;   // per-user random, 32 bytes
     std::vector<char> passwordHash;   // now Argon2id, not BLAKE2b
 };
```

Hash function migration in
[Sodium.cpp](../src/SSVOpenHexagon/Online/Sodium.cpp):
new `sodiumPasswordHash(plaintext, salt) -> std::vector<char>` using
`crypto_pwhash_argon2id` with moderate parameters (`OPSLIMIT_MODERATE`,
`MEMLIMIT_MODERATE`). The old `sodiumHash` stays for non-password
hashes.

Two new packet types in
[Online/Shared.hpp](../include/SSVOpenHexagon/Online/Shared.hpp):

```cpp
struct CTSPResetPasswordWithSteamTicket
{
    U64                steamId;
    std::vector<U8>    ticket;            // Steam encrypted app ticket
    std::vector<char>  newPasswordHash;
    std::vector<char>  newPasswordSalt;
};

struct STCPPasswordResetResult
{
    bool        ok;
    std::string error;   // empty on ok
};
```

Server flow: validate ticket against the supplied Steam ID, find the
user row by steamId, replace its salt+hash. **No login token issued
yet** -- user logs in normally with the new password. The ticket
validates "this is the real owner of that Steam ID at this moment."

Migration of existing users: opportunistic. On successful login with
the old hash, re-hash with Argon2id and store. Old hashes co-exist
during the transition (a `version` byte in `passwordHash` would be
sufficient), and after a deprecation window we can refuse them.

### 5.4 F4 -- Profile collapse

Drop the "multiple local profiles" feature entirely. Replace
[`HGAssetsImpl::profileDataMap`](../src/SSVOpenHexagon/Global/Assets.cpp#L84)
+ `currentProfilePtr` with a single
`offlineProfile: ProfileData` loaded from `Profiles/offline.json`. On
upgrade, if any old per-name profile files exist, merge them
non-destructively into `offline.json` and rename the old files to
`*.bak`.

Replace
[`pSetCurrent`/`getCurrentLocalProfile`](../src/SSVOpenHexagon/Global/Assets.cpp#L1421-L1423)
with `getOfflineProfile()`. Online play is no longer "a different
profile"; it's a layer on top of offline that, when logged in,
*also* uploads scores and *can* download remote scores into a
separate `onlineScores` map (kept distinct so going offline doesn't
clobber the user's local best).

The favourites set stays where it is in `ProfileData::favoriteLevelDataIDs`
([ProfileData.hpp:25](../include/SSVOpenHexagon/Data/ProfileData.hpp#L25)),
just on the single offline profile.

The "profile picker" screen disappears. The Main menu's "Profile"
entry becomes a small status panel (offline name + login state +
total play time) with no picker.

### 5.5 F5 -- Real level preview

Add to [`HexagonGame`](../include/SSVOpenHexagon/Core/HexagonGame.hpp):

```cpp
// Runs the level for `seconds` of in-game time at `difficultyMult`,
// stepping the simulation at a fixed rate, drawing each frame into
// the supplied render texture. Caller can sample frames (e.g. one
// every 50 ms) or just take the last frame as a thumbnail.
void renderPreviewSequence(const sf::base::String& levelId,
                           float                   difficultyMult,
                           float                   seconds,
                           sf::RenderTexture&      target,
                           sf::base::FixedFunction<void(float t), 64> onFrame);
```

The infrastructure exists -- `runReplayUntilDeathAndGetScore`
already drives the engine headlessly for replay validation. The
preview helper reuses the same "run engine without a player" path
but writes frames to a render texture instead of computing a score.

The level-select screen calls `renderPreviewSequence` once per
focus change (so once per arrow-key press, not per frame), with
`seconds = 4.f` and `onFrame` capturing every Nth frame into a
small ring buffer of textures that the UI then plays back as a
short looping clip. **No persistent cache** -- when the user moves
focus, the previous textures are simply overwritten.

If `renderPreviewSequence` turns out to be too slow on cold pack
load (Lua compile etc.), we can pre-warm one level per pack on
pack-list-version change. Premature; only do if measured.

### 5.6 F6 / F7 -- Search, filter, sort

Three small additions:

1. **`LevelData` gains an optional `tags` field** (parsed from
   `level.json`, empty if absent). This is the only data extension
   needed; `name`/`author`/`difficultyMults` are already present.
2. **`ProfileData` gains a per-level state map**:
   ```cpp
   struct PerLevelState
   {
       sf::base::U64 lastPlayedTs   = 0;
       sf::base::U32 playCount      = 0;
       bool          isCompleted    = false;
   };
   std::unordered_map<sf::base::String, PerLevelState> perLevelState;
   ```
   This is where "last played" / "play count" sort keys come from.
3. **The level-select screen owns a filtered list**, derived on
   demand:
   ```cpp
   void rebuildFilteredLevels(App::LevelSelectScreen& s,
                              const HGAssets&         assets,
                              const ProfileData&      profile);
   ```
   Called when search text, sort key, filter spec, or
   `assets.packListVersion()` changes. Stores the resulting
   `Vector<LevelId>` in `s.filteredLevelIds`. The renderer iterates
   that. **No per-frame sort.**

Search is plain `case-insensitive substring match` against
`name + author + description`. We don't ship a fuzzy matcher in v1
-- if we measure that a real Steam-Workshop-scale catalogue makes
substring search uncomfortable, we add trigrams later.

Sort keys (small enum, plain switch in the comparator):
`PackPriority` (default), `Name`, `Author`, `MinDifficulty`,
`PersonalBest`, `LastPlayed`, `PlayCount`, `Random`.

Filter spec (small struct, plain `&&` chain):

```cpp
struct FilterSpec
{
    bool                                  favoritesOnly = false;
    bool                                  uncompletedOnly = false;
    sf::base::Optional<float>             minDifficulty;
    sf::base::Optional<float>             maxDifficulty;
    sf::base::Optional<sf::base::String>  packIdEq;
    sf::base::Vector<sf::base::String>    requiredTags;
};
```

### 5.7 F8 -- Favourites discoverability

No backend change; the data already exists. The new level-select
screen draws a star icon at the right edge of every level row
(filled if favourited, empty otherwise) and a "★ Favourites only"
toggle in the filter bar. Mouse-clicking the star toggles; the F1
hotkey stays as a shortcut for keyboard users.

---

## 6. Module layout

New files only. The old `MenuGame` / `MenuSystem` stay untouched
during the transition.

```
include/SSVOpenHexagon/UI/
  UI.hpp                  -- primitives + Context (section 2)
  App.hpp                 -- App struct + Screen enum (section 3)
  Screens.hpp             -- declarations of drawXxxScreen() functions

src/SSVOpenHexagon/UI/
  UI.cpp                  -- primitive implementations
  Screens/
    Main.cpp              -- drawMainMenu
    Options.cpp           -- drawOptions
    LevelSelect.cpp       -- drawLevelSelect + rebuildFilteredLevels
    WorkshopBrowse.cpp    -- drawWorkshopBrowse + Steam event handling
    Online.cpp            -- login, registration, password recovery
    Profile.cpp           -- offline status panel
    Loading.cpp / Epilepsy.cpp -- trivial
```

Backend additions go in their existing files
([Steam.cpp](../src/SSVOpenHexagon/Core/Steam.cpp),
[Assets.cpp](../src/SSVOpenHexagon/Global/Assets.cpp),
[HexagonServer.cpp](../src/SSVOpenHexagon/Core/HexagonServer.cpp),
[HexagonClient.cpp](../src/SSVOpenHexagon/Core/HexagonClient.cpp),
[Sodium.cpp](../src/SSVOpenHexagon/Online/Sodium.cpp),
[DatabaseRecords.hpp](../include/SSVOpenHexagon/Online/DatabaseRecords.hpp)).

---

## 7. Coexistence with the old `MenuGame`

We don't try to replace everything in one PR. The transition strategy:

1. **Phase 0 -- primitives + Main screen.**
   Build `UI.hpp` / `UI.cpp` and `Main.cpp`. Add a build flag
   `SSVOH_NEW_UI=on` that, when set, routes the existing
   `States::SMain` to `drawMainMenu` instead of `MenuGame::drawMainMenu`.
   Both code paths exist; we can A/B by toggling the flag. Old code
   untouched.
2. **Phase 1 -- Options + Profile screens.**
   Same pattern. `States::MOpts` and `States::SLPSelect` route to
   the new screens when the flag is on.
3. **Phase 2 -- Level Select.**
   The hardest screen. Implement `drawLevelSelect` with search /
   filter / sort / favourites. Backend: ship the `LevelData::tags`
   and `ProfileData::perLevelState` extensions.
4. **Phase 3 -- Workshop Browse + hot install.**
   Backend: extend `Steam::steam_manager` (5.1), add
   `HGAssets::installPackAtRuntime` (5.2). UI: `WorkshopBrowse.cpp`.
5. **Phase 4 -- Online + password recovery.**
   Backend: schema migration (5.3), Argon2id, two new packet types,
   server endpoint. UI: replaces the existing online/login menu.
6. **Phase 5 -- preview engine.**
   Backend: `HexagonGame::renderPreviewSequence` (5.5). UI: hook it
   into `drawLevelSelect`.
7. **Phase 6 -- delete the old code.**
   Once every state is routed through the new system, delete
   `MenuGame.cpp`, `MenuGame.hpp`, `BindControl.{hpp,cpp}`,
   `include/SSVOpenHexagon/MenuSystem/`, and remove
   `extlibs/SSVMenuSystem/` from the submodules / CMakeLists.

Each phase ships independently and the build is green at every step.

---

## 8. What we're explicitly **not** doing

To keep KISS honest, here's the list of things the design *omits* on
purpose. If reviewing this and tempted to add any of them, push back
until we have a measured need.

- **No widget IDs / hash IDs.** Each screen tracks focus by index.
- **No layout DSL / constraints.** Layout is a cursor + a few helpers.
- **No theme inheritance / push-pop.** Theme is a struct; mutate it
  directly.
- **No animation framework.** If a screen needs animation, it's a
  float on its sub-struct that the screen lerps in its draw call.
- **No deferred rendering / draw command list.** Every widget calls
  `target.draw(...)` immediately.
- **No allocations per frame.** `Input` and `Context` are fixed-size;
  string buffers in `App` are reused, not reallocated.
- **No per-pack worker thread for installs.** Synchronous install on
  the main thread; revisit only if a measured stall exceeds one
  frame.
- **No texture cache for level previews.** Rebuilt on focus change.
- **No fuzzy search.** Substring match in v1.
- **No email-based password recovery.** Steam-ticket auth is the
  recovery channel; we don't run an SMTP server.
- **No multi-profile support.** One offline profile per machine.
  The collapse is a *feature*.
- **No undoable settings stack.** Settings apply immediately and
  persist on screen exit.
- **No multi-language i18n in v1.** English only; the data path is
  `sf::base::StringView` so adding translation later is a string-table
  swap, not an architecture change.

---

## 9. Decisions on open questions

(Resolved by author 2026-04-26.)

1. **Input primacy:** keyboard/joystick stays primary (reflex game).
   Mouse must be supported much more cleanly than today -- every
   selectable surface is hover-highlightable, every action has a
   click target, click-to-focus is consistent. No "mouse only as an
   afterthought" idiom (the current `mustChangeIndexTo` flag pattern
   in [MenuGame.cpp:2637-2690](../src/SSVOpenHexagon/Core/MenuGame.cpp#L2637-L2690)
   does not survive the rewrite).
2. **Workshop browse:** Open Hexagon's Workshop catalogue is in the
   hundreds -- we don't need infinite scroll. Two preset views (most
   popular, newest), a "browse all" pagination view, and a search-by-
   name input. Defaults to "most popular" on entry.
3. **Profile merge:** silent union into `offline.json` on first run
   after upgrade. Old profile files renamed to `*.bak` and left on
   disk for the user to consult / delete manually.
4. **Preview rendering:** keep the existing colours-only background
   for v1 (cheap, already works). Architect the preview path so a
   live-engine `RenderTexture` can replace it later without
   re-architecting the screen. Concretely: the preview is rendered
   by a small `drawLevelPreview(target, levelStatus, t)` function
   the screen calls once per frame; v1 implementation is "draw the
   coloured hexagon"; v2 swap-in is "tick a hidden `HexagonGame`
   into a `RenderTexture` and blit." Try the full live render first
   -- it may be cheap enough to skip the v1 fallback entirely.
5. **Pack dependencies on hot install:** auto-subscribe missing
   dependencies. Workshop packs that declare dependencies trigger
   a recursive subscribe + install on the parent install path.

---

## 10. Animations and smooth scrolling

Glossed over in v1 of this doc -- added on review. The current UI's
fold/unfold/scroll polish is part of the game's identity and the
rewrite has to preserve it without reintroducing the state-fragmentation
mess
([research §3.6](UI_REWRITE_RESEARCH.md#36-member-variable-inventory-of-menugame)
counts seven separate `*Offset` fields scattered across `Category`,
`ItemBase`, and screen-specific structs).

### 10.1 Approach

Animations are **plain floats on App sub-structs**, lerped each frame
by a small set of easing helpers. No animation framework, no
`AnimatedFloat<>` template, no callbacks. The float lives where it
"obviously belongs" -- `App::MainScreen::selectionY` for the main
menu's selection cursor, `App::LevelSelectScreen::scrollY` for the
level list scroll, `App::LevelSelectScreen::detailsOffset` for the
right-panel slide-in.

### 10.2 Helpers in `ohui`

```cpp
namespace ohui {

// Exponential-ease toward target. `dt` is seconds. `speed` is the
// inverse time constant (12.f ≈ "reach target in ~80 ms"). Returns
// true if a step happened, so callers can skip work when settled.
bool stepToward(float& current, float target, float dt, float speed = 12.f);
bool stepToward(sf::Vector2f& current, sf::Vector2f target, float dt, float speed = 12.f);

// Pure easing functions (operate on t in [0,1], return [0,1]).
float easeOutCubic(float t);
float easeInOutCubic(float t);
float easeOutBack(float t);   // small overshoot -- used for selection bumps

}
```

That's the entire animation surface. Anything more complex is just
"interpolate this float toward that target with this speed" written
in the screen function.

### 10.3 Conventions

- **Selection cursor**: each screen with a list keeps a
  `selectionY: float` separate from `selectedIdx: int`. Each frame
  `stepToward(selectionY, idxToY(selectedIdx), dt)`. The cursor visual
  is drawn at `selectionY`, not at `selectedIdx`.
- **Page transitions**: when a screen change happens, the outgoing
  screen's `transitionAlpha` ramps 1→0 over ~150 ms while the new
  screen's ramps 0→1. While `< 1`, draw both. Owned by `App` itself,
  not per-screen.
- **Element entry**: when a list of items appears, each item gets a
  small staggered alpha. Implemented inline in the screen with a
  single `float openProgress` per screen, lerped 0→1 when entered;
  per-item alpha = `clamp(openProgress * N - i, 0, 1)`.
- **No animation interruption logic.** If the user changes selection
  mid-animation, the target updates and the same `stepToward` keeps
  going. No "cancel current tween, start new one" -- exponential ease
  handles target changes naturally.

### 10.4 Performance

Per-frame animation cost is a handful of `stepToward` calls -- single-
digit operations, no allocations. We `clear()` and refill no buffers
(immediate-mode), so animations don't introduce a separate per-frame
cost beyond the lerps themselves.

When all animations on a screen are settled (every `stepToward`
returns false), the screen is in a "stable" state. We don't do
anything special with this -- we still redraw every frame, which is
the immediate-mode philosophy. If profiling later shows menu redraw
is wasteful, we revisit.

---

*End of design. Phase 0 implementation begins.*

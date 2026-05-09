# UI rewrite -- research document

Status: **research only**, no design or code decisions yet.
Audience: anyone designing or implementing the new UI.
Goal: fully describe what's there today, what's wrong with it, and **why** at the implementation level -- so the rewrite can address root causes rather than symptoms.

---

## 1. Executive summary

SSVOpenHexagon's UI is a single 5,900-line `MenuGame` god class
([src/SSVOpenHexagon/Core/MenuGame.cpp](../src/SSVOpenHexagon/Core/MenuGame.cpp))
sitting on top of a small retained-OOP "menu system" library
([extlibs/SSVMenuSystem/](../extlibs/SSVMenuSystem/) +
[include/SSVOpenHexagon/MenuSystem/](../include/SSVOpenHexagon/MenuSystem/)).
Every screen has its own bespoke render path with hard-coded coordinates;
input is dispatched through long state-branching `*Action()` cascades; the
"menu item" abstraction is OOP-heavy but does almost nothing useful (no
rendering, no layout, no input -- just `exec()`/`getName()`).

The user-visible flaws (no in-game workshop install, restart-to-load, no
password recovery, stale local/online profile split, weak preview, no
search/filter/sort, no favorites UI beyond a hidden hotkey toggle) are not
*UI* problems alone -- most of them are blocked by **backend invariants**:

- `HGAssets` is a one-shot constructor: levels/packs/styles/music are
  loaded once at game start, baked into static maps, never re-scanned.
- `Steam::steam_manager` exposes only read-only Workshop APIs
  (`GetSubscribedItems` / `GetItemInstallInfo`) -- there is no
  subscribe/unsubscribe/download-progress wiring at all.
- Passwords are hashed client-side with `crypto_generichash` (BLAKE2b,
  unsalted, no work factor) and the database has no recovery columns.
- The profile system is filesystem-only; `currentProfilePtr` is a single
  pointer assumed alive for the whole session, and there is no link
  between local profiles and online accounts.

The rewrite needs to address both layers -- UI *and* backend -- together.
This document is the inventory; design follows.

---

## 2. Scope

What's in: every UI screen, the entire `MenuSystem/` hierarchy, the level
selection panel and its preview, the asset/profile/Steam/login backends
that the UI talks to, and the listed flaws plus everything we found
adjacent to them.

What's out: rendering of the actual gameplay (`HexagonGame`), Lua
scripting hooks, achievements/Discord/Steam stats. Those are touched only
where the UI calls into them.

---

## 3. UI architecture today

### 3.1 The `MenuGame` god class

[MenuGame.cpp](../src/SSVOpenHexagon/Core/MenuGame.cpp) is **5,903 lines**
and the header [MenuGame.hpp](../include/SSVOpenHexagon/Core/MenuGame.hpp)
declares ~80 member variables across ~600 lines. Every UI screen, every
animation, every input cascade lives in this one class. Concrete
responsibilities mixed in:

- State-machine driver (which screen is active)
- Per-screen rendering (one bespoke method per screen)
- Animation interpolation (offsets, scrolls, fold/unfold)
- Input dispatch (keyboard, mouse, joystick, gamepad, text entry)
- Lua/asset/profile coordination
- Dialog/popup orchestration
- First-time-tip overlays

It calls into `HGAssets`, `HexagonClient`, `Audio`, `LeaderboardCache`,
`HexagonDialogBox`, `Lua::LuaContext`, the Steam manager, the Discord
manager -- there is no façade in front of any of these.

### 3.2 The `MenuSystem/` OOP wrapper

The repo has **two copies** of `SSVMenuSystem`:

- [extlibs/SSVMenuSystem/include/SSVMenuSystem/](../extlibs/SSVMenuSystem/include/SSVMenuSystem/)
  -- the upstream library (separate git submodule).
- [include/SSVOpenHexagon/MenuSystem/](../include/SSVOpenHexagon/MenuSystem/)
  -- a vendored fork with a few local modifications
  ([`SSVMenuSystem.hpp`](../include/SSVOpenHexagon/MenuSystem/SSVMenuSystem.hpp)
  differs from the upstream copy and the `Global/` directory is dropped).

The fork is what's actually used; the `extlibs/` copy is unused dead
weight. **Both can be deleted by the rewrite.**

The hierarchy:

| Class | File | Role |
|---|---|---|
| `Menu` | [Menu/Menu.hpp](../include/SSVOpenHexagon/MenuSystem/Menu/Menu.hpp) | Owns N `Category` and a stack of "previous categories" for back-navigation |
| `Category` | [Menu/Category.hpp](../include/SSVOpenHexagon/MenuSystem/Menu/Category.hpp) | Owns N `ItemBase`, has `index` + `offset` for animation |
| `ItemBase` | [Menu/ItemBase.hpp](../include/SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp) | Virtual base: `exec()`, `increase()`, `decrease()`, `getName()` |
| `Single` | [Items/Single.hpp](../include/SSVOpenHexagon/MenuSystem/Items/Single.hpp) | Button -- calls a `FixedFunction<void()>` on `exec()` |
| `Toggle` | [Items/Toggle.hpp](../include/SSVOpenHexagon/MenuSystem/Items/Toggle.hpp) | On/off -- `getName()` appends `: on`/`: off` |
| `Slider` | [Items/Slider.hpp](../include/SSVOpenHexagon/MenuSystem/Items/Slider.hpp) | Numeric range -- `increase()`/`decrease()` mutate |
| `Goto` | [Items/Goto.hpp](../include/SSVOpenHexagon/MenuSystem/Items/Goto.hpp) | Navigates to a sub-category |
| `GoBack` | [Items/GoBack.hpp](../include/SSVOpenHexagon/MenuSystem/Items/GoBack.hpp) | Pops the navigation stack |
| `KeyboardBindControl`, `JoystickBindControl` | [Core/BindControl.hpp](../include/SSVOpenHexagon/Core/BindControl.hpp) | Custom `ItemBase` subclasses for "press a key to bind" |

**Key observation:** the virtual surface is *only*
`exec()`/`increase()`/`decrease()`/`getName()`. Items have no `draw()`,
no `getSize()`, no input handlers. The entire **rendering and layout
happens in `MenuGame`**, which calls `items[i]->getName()` in a loop and
draws strings at hand-computed coordinates. The OOP indirection buys
almost nothing -- five item types essentially encode "what `exec` does"
and "what string `getName` returns." A flat tagged union (or just plain
data + a switch in the renderer) would express this with strictly less
machinery.

### 3.3 Inventory of screens / states

[MenuGame.hpp:69-81](../include/SSVOpenHexagon/Core/MenuGame.hpp#L69-L81)
defines the `States` enum:

| State | Screen | Render entry point |
|---|---|---|
| `ETLPNewBoot` / `ETLPNew` | "Enter player name" text-entry overlays | `drawEnteringText()` ([MenuGame.cpp:4016](../src/SSVOpenHexagon/Core/MenuGame.cpp#L4016)) |
| `LoadingScreen` | Pack/level loading + error log | inline in `draw()` |
| `EpilepsyWarning` | Statutory warning splash | inline in `draw()` |
| `SLPSelectBoot` / `SLPSelect` | Local-profile picker | `drawProfileSelection()` ([MenuGame.cpp:3815](../src/SSVOpenHexagon/Core/MenuGame.cpp#L3815)) |
| `SMain` | Main menu | `drawMainMenu()` ([MenuGame.cpp:3653](../src/SSVOpenHexagon/Core/MenuGame.cpp#L3653)) |
| `MOpts` | Options hierarchy (Gameplay, Controls, Resolution, Graphics, Audio, Advanced) | `drawSubmenusSmall()` |
| `MOnline` | Online/login/leaderboards | `drawSubmenusSmall()` |
| `LevelSelection` | Pack + level browser | `drawLevelSelectionLeftSide()` + `drawLevelSelectionRightSide()` ([MenuGame.cpp:5127–5709](../src/SSVOpenHexagon/Core/MenuGame.cpp#L5127)) |

Plus an in-screen `HexagonDialogBox` overlay drawn last in `draw()`,
synchronised via `dialogBoxDelay` and `ignoreInputs` counters.

`getCurrentMenu()`
([MenuGame.cpp:170-190](../src/SSVOpenHexagon/Core/MenuGame.cpp#L170-L190))
maps each state to one of `mainMenu` / `optionsMenu` / `onlineMenu` /
`profileSelectionMenu` -- except for level selection, loading, and
epilepsy, which are bespoke (no menu object backing them).

### 3.4 Rendering pipeline

- **Retained-mode SFML graphics**: pre-allocated `sf::Text` instances
  (16 of them, in `MenuFont` structs at
  [MenuGame.hpp:299-316](../include/SSVOpenHexagon/Core/MenuGame.hpp#L299))
  whose positions are recomputed every frame; two
  `FastVertexVector<Triangles>` (`menuBackgroundTris`, `menuQuads`) that
  are `clear()`'d and re-filled each frame.
- **Three view layers**: `backgroundCamera` (rotating hexagon),
  `overlayCamera` (UI), screen-space (final overlay). Rendered via
  template helpers `drawBackground()` / `drawOverlay()` / `drawScreen()`.
- **No widget / immediate-mode abstraction**: every screen has its own
  big bespoke draw method. `drawLevelSelectionRightSide()` is **317
  lines**, `drawLevelSelectionLeftSide()` is **394 lines**,
  `drawProfileSelection()` is **121 lines**. They duplicate quad layout,
  scroll math, mouse-overlap detection.
- **Hard-coded layout arithmetic everywhere**: indents (`indentBig=400`,
  `indentSmall=540`), aspect-ratio fallbacks (`fourByThree` boolean
  branch), animation magic numbers (`baseScrollSpeed=30.f`).
- **No shaders for menus.** The hexagon background uses
  `StyleData::drawBackgroundMenu` (style-driven colors, plain
  triangles).

### 3.5 Input handling

Flow:

1. [GameWindow](../include/SSVOpenHexagon/GameSystem/GameWindow.hpp)
   pumps SFML events into an `InputState` bitset.
2. `MenuGame::update(float)`
   ([MenuGame.cpp:2490-2893](../src/SSVOpenHexagon/Core/MenuGame.cpp#L2490),
   ~400 lines) polls keyboard/mouse/joystick state and queues actions.
3. `upAction()`, `downAction()`, `leftAction()`, `rightAction()`,
   `okAction()`, `exitAction()`
   ([MenuGame.cpp:1909-2437](../src/SSVOpenHexagon/Core/MenuGame.cpp#L1909))
   are large `switch (state)` cascades that decide what each direction
   means *in this state*. `upAction()` alone is ~100 lines branching on
   `packChangeState`, `focusHeld`, `isInMenu()`.
4. Mouse hover/click goes through deferred-action flags
   (`mustChangeIndexTo`, `mustFavorite`, `mustPlay`,
   `mustChangePackIndexTo`, `mustUseMenuItem`) -- set by hover detection,
   processed at top of next `update()`. There is no unified event queue;
   each feature has its own flag.
5. `KeyboardBindControl` / `JoystickBindControl` enter "consume next
   event" mode by overriding `exec()` and stealing the input stream.

### 3.6 Member-variable inventory of `MenuGame`

Loosely grouped (all from
[MenuGame.hpp](../include/SSVOpenHexagon/Core/MenuGame.hpp)):

- **External systems** (~13): `steamManager`, `discordManager`, `assets`,
  `audio`, `game`, `window`, `hexagonClient`, `dialogBox`,
  `leaderboardCache`, `lua`, fonts, `currentPack`.
- **Active state machine**: `state`, `lastUpperCaseState`,
  `nextState`, plus the three "must show first-time tip" booleans.
- **Five menu objects**: `welcomeMenu`, `mainMenu`, `optionsMenu`,
  `onlineMenu`, `profileSelectionMenu`.
- **16 cached `MenuFont` text objects.**
- **Two vertex vectors** (`menuBackgroundTris`, `menuQuads`).
- **Mouse interaction state** (~9 booleans/ints): `lastMouseClick`,
  `mouseHovering`, `mouseWasPressed`, `mousePressed`, `mustFavorite`,
  `mustPlay`, `mustChangeIndexTo`, `mustChangePackIndexTo`,
  `mustUseMenuItem`, `mouseCursorVisible`, `lastMouseMovedPosition`.
- **Level-selection sub-state** (~30): `lvlSlct` and `favSlct`
  `LevelDrawer` structs each with `packIdx`, `currentIndex`, `YOffset`,
  `YScrollTo`, `lvlOffsets`, etc., plus `diffMultIdx`, `firstLevelSelection`,
  `packChangeState`, `namesScroll[]`, `levelDescription`, layout metrics
  (`textToQuadBorder`, `slctFrameSize`, `packLabelHeight`,
  `levelLabelHeight`), `packChangeOffset`, `levelDetailsOffset`,
  `scrollSpeed`, `baseScrollSpeed`, `difficultyBumpEffect`.
- **Animation state**: `enteringTextOffset`, transient menu offsets,
  `wheelProgress`, `touchDelay`.
- **Misc transient**: `enteredChars` (text entry buffer),
  `mustShowLoginAtStartup`, `dialogBoxDelay`, `ignoreInputs`, `strBuf`,
  ~9 colors.

This is an enormous accumulator of accidental state -- the kind of class
that signals the whole abstraction was wrong from the start.

---

## 4. Backend constraints

Every user-visible flaw has a backend root cause. This section
documents the backend layer; section 5 maps each flaw to its blocker.

### 4.1 Steam Workshop integration -- read-only and one-shot

[Steam.cpp](../src/SSVOpenHexagon/Core/Steam.cpp) /
[Steam.hpp](../include/SSVOpenHexagon/Core/Steam.hpp) expose a tiny
read-only Workshop surface:

- `load_workshop_data()`
  ([Steam.cpp:189-232](../src/SSVOpenHexagon/Core/Steam.cpp#L189-L232))
  calls `SteamUGC()->GetSubscribedItems` + `GetItemInstallInfo` once,
  writes the resulting folder list to `workshopCache.json`, populates
  `_workshop_pack_folders` (a `std::unordered_set<sf::base::String>`).
- `for_workshop_pack_folders()`
  ([Steam.cpp:628-640](../src/SSVOpenHexagon/Core/Steam.cpp#L628-L640))
  iterates that cached set.
- That's it. **No `subscribe_workshop_item`, no `unsubscribe`, no
  `download_progress`, no `is_installed`.** Grep for `SubscribeItem` in
  Steam.cpp returns nothing.

Callbacks wired up
([Steam.cpp:105-107](../src/SSVOpenHexagon/Core/Steam.cpp#L105-L107)):
only `UserStatsReceived_t`, `UserStatsStored_t`,
`UserAchievementStored_t` -- none of `ItemInstalled_t`,
`SubscribedItemsListChanged_t`, `DownloadItemResult_t`.

The CLI tool [src/OHWorkshopUploader/main.cpp](../src/OHWorkshopUploader/main.cpp)
*does* exercise the upload-side Workshop API
(`CreateItem`/`StartItemUpdate`/`SetItemContent`/`SubmitItemUpdate`)
but it's a separate binary used by content creators; the main game
binary has no integration with it.

The result: a player **must** subscribe to a workshop item via the
Steam Client UI, **must** wait for Steam to download it externally, and
**must then restart Open Hexagon** for it to show up in the level list.
The "restart" requirement is enforced by the next bullet.

### 4.2 Asset / level loading -- one-shot and never re-scanned

[`HGAssets`](../include/SSVOpenHexagon/Global/Assets.hpp) is constructed
exactly once in `main.cpp` and lives until program exit.
[`HGAssetsImpl::HGAssetsImpl`](../src/SSVOpenHexagon/Global/Assets.cpp#L293-L369)
calls `loadAllPackDatas()` → `loadAllPackAssets()` → for each pack
loads shaders, music, styles, levels, and Lua scripts. The results are
baked into static maps:

- `packDatas`, `levelDatas`, `levelDataIdsByPack`, `styleDataMap`,
  `musicDataMap`, `assetStorage`.

After the constructor returns, **no code path re-scans the pack
directory or re-invokes `loadAllPackDatas()`**. New `pack.json` files
appearing on disk are invisible until the next process start.

There *is* a per-pack reload helper --
`HGAssets::reloadPack()`/`reloadLevel()`
([Assets.cpp:1133+](../src/SSVOpenHexagon/Global/Assets.cpp#L1133)) --
that updates `levelDatas`/`styleDataMap`/`musicDataMap` in place. It's
used for development hot-reload (likely a Lua console hook), **not**
called by the gameplay or Workshop paths.

The other invariant baked at load time is **inter-pack dependencies**
([Assets.cpp:415-442](../src/SSVOpenHexagon/Global/Assets.cpp#L415-L442))
-- a pack referencing another pack is resolved at scan time. A new pack
that depends on a not-yet-loaded pack would fail validation under any
naïve runtime-add path; the rewrite has to handle the dependency graph
explicitly.

### 4.3 Password storage -- weak hash, no recovery

Client-side: `HexagonClient::sendRegister`/`sendLogin`
([HexagonClient.cpp:270-297](../src/SSVOpenHexagon/Core/HexagonClient.cpp#L270-L297))
take a `passwordHash` already-hashed by the caller (`saltAndHashPwd`).
The hash function used is `sodiumHash`
([Sodium.cpp:68-83](../src/SSVOpenHexagon/Online/Sodium.cpp#L68-L83)),
which calls libsodium's `crypto_generichash` (BLAKE2b). This is **not**
a password-hashing function:

- No salt parameter (the surrounding `saltAndHashPwd` adds a single
  hard-coded salt, identical for every user).
- No work factor -- runs as fast as the CPU can.
- Identical inputs produce identical outputs across the user base, so a
  rainbow-table attack against the leaked hashes is trivial.

Server-side:
[`Database::User`](../include/SSVOpenHexagon/Online/DatabaseRecords.hpp#L15-L21)
schema has `id`, `steamId`, `name`, `passwordHash`. **No
`recovery_token`, no `email`, no `password_reset_at`.** `Database.cpp`
has no recovery code path. `HexagonServer.cpp` defines no
"reset password" packet type, no email integration, no token issuance.

So when a user forgets their password, the only way back is for the
operator to run an SQL update against `ohdb.sqlite`. There is no UX
path *and* no server endpoint *and* no schema column to support
recovery.

### 4.4 Profile system -- filesystem-only, single-pointer, no sync

[`ProfileData`](../include/SSVOpenHexagon/Data/ProfileData.hpp#L19-L48)
is a `name` + `scores: unordered_map<levelId, float>` +
`favoriteLevelDataIDs: unordered_set<levelId>` + `version`. Stored as
one `.json` per profile in `Profiles/`.

`HGAssetsImpl` holds `profileDataMap: std::map<String, ProfileData>` of
all loaded profiles plus `currentProfilePtr: ProfileData*` -- a **single
raw pointer to whichever one is "active"**
([Assets.cpp:84-85](../src/SSVOpenHexagon/Global/Assets.cpp#L84)).

Online play uses a *separate* identity: `HexagonClient` logs in with
Steam ID + password hash and the server stores scores keyed by Steam
ID, **not by profile name**. Local-profile favorites and scores are
*never* uploaded; remote scores are *never* downloaded into the local
profile. The two namespaces are wholly independent.

The "multiple local profiles" feature buys very little -- they all run
on the same Steam ID, so they're effectively just labelled save slots
on one machine. Collapsing them into a single offline namespace would
lose: the ability to share one PC between roommates without overwriting
each other's offline scores. That's the entire feature.

---

## 5. Catalogue of flaws -- root causes

For each user-stated flaw: where it shows up, what blocks it, what the
rewrite has to change.

### Flaw F1 -- Cannot browse / download / install Workshop levels in-game

**Symptom:** the user must alt-tab to Steam, find the Open Hexagon
Workshop page, click subscribe, wait, alt-tab back.

**Root cause:** `Steam::steam_manager` exposes only
`for_workshop_pack_folders()` (read-only iteration over an already-
subscribed set). No `SubscribeItem`, `UnsubscribeItem`, `DownloadItem`,
or `RequestUGCDetails` calls; no callbacks for `ItemInstalled_t` or
`SubscribedItemsListChanged_t`.

**Backend changes needed:**

1. Wrap `SteamUGC()->SubscribeItem` / `UnsubscribeItem` /
   `DownloadItem` and expose them on `steam_manager`.
2. Wrap `SteamUGC()->CreateQueryAllUGCRequest` (or
   `CreateQueryUserUGCRequest`) for browsing the catalogue from inside
   the game.
3. Register `RemoteStoragePublishedFileSubscribed_t`,
   `RemoteStorageUnsubscribePublishedFileResult_t`,
   `DownloadItemResult_t`, `ItemInstalled_t` callbacks; expose them as
   a small async-event channel the UI can poll.
4. Hook the install callback to feed the new pack into asset loading
   (see F2).

**UI changes needed:** a Workshop-browse screen with thumbnail/description/
size/subscribe button per item; a "downloading" progress indicator.

### Flaw F2 -- Must close/reopen game to load newly downloaded levels

**Symptom:** even if the user does subscribe via the Steam Client and
the pack downloads, it does not appear until the next game launch.

**Root cause:** `HGAssets` is constructed once. Its maps
(`packDatas`, `levelDatas`, `levelDataIdsByPack`, `styleDataMap`,
`musicDataMap`, `selectablePackInfos`) are never re-scanned, never
augmented at runtime. The existing `reloadPack()`/`reloadLevel()`
helpers ([Assets.cpp:1133+](../src/SSVOpenHexagon/Global/Assets.cpp#L1133))
exist for hot-reload during development but are not wired to any
runtime entry point and don't handle "first-time install" (only
re-scan of already-loaded packs).

**Backend changes needed:**

1. Generalise `reloadPack()` into `installPackAtRuntime(folderPath)`
   that handles "pack not yet present in maps" (validate `pack.json`,
   resolve dependencies against currently-loaded packs, populate every
   relevant map, append to `selectablePackInfos`, sort).
2. Mirror an `uninstallPack(packId)` that removes from every map and
   clears any references in profiles' favorites/scores carefully (or
   leaves them as orphans the UI hides -- design call).
3. Either a notification API `HGAssets::onPackInstalled(callback)` or a
   "dirty" flag the menu re-reads each frame.
4. Audit Lua state: per-level Lua is parsed at `loadPackAssets`
   ([Assets.cpp:464-527](../src/SSVOpenHexagon/Global/Assets.cpp#L464-L527));
   it should re-parse cleanly for a newly-installed pack since each
   level gets its own context, but verify there are no globals shared
   across packs that need re-init.

### Flaw F3 -- No password recovery

**Symptom:** forgotten password = email the developer, who runs SQL
manually.

**Root cause (compound):**

1. `Database::User` schema has no recovery columns
   ([DatabaseRecords.hpp:15-21](../include/SSVOpenHexagon/Online/DatabaseRecords.hpp#L15-L21)).
2. `HexagonServer` defines no recovery-related packet types in
   [Online/Shared.hpp](../include/SSVOpenHexagon/Online/Shared.hpp) and
   no token-issuance logic.
3. Even the basic password handling is weak -- `crypto_generichash` is
   too fast for password hashing; the salt
   ([HexagonClient.cpp:saltAndHashPwd](../src/SSVOpenHexagon/Core/HexagonClient.cpp))
   is global rather than per-user. The rewrite is a good time to fix
   this even before adding recovery.

**Backend changes needed:**

1. Migrate password storage to `crypto_pwhash_argon2id` with a
   per-user random salt stored alongside the hash.
2. Add columns to `User`: `password_salt` (replaces the global one),
   `email` (optional), `recovery_token`, `recovery_token_expires_at`.
3. New packet types: `CTSPRequestPasswordReset { steamId, email? }`,
   `CTSPSubmitPasswordReset { token, newPasswordHash, newSalt }`,
   `STCPPasswordResetIssued`, `STCPPasswordResetResult`.
4. Server endpoint: generate a 32-byte random token, store hash of
   token (not raw) with 1-hour expiry, ship token to user (display
   in-game if Steam ID is the recovery channel, or send via email if
   we add SMTP).

**Note:** because Steam ID is already the primary key on the user
table, the simplest "recovery" channel is "log in via Steam ticket
once, then set a new password" -- no email needed if we trust Steam's
auth.

### Flaw F4 -- Local/online profile dichotomy

**Symptom:** local progress and online progress are unrelated; the user
maintains two separate sets of scores, and the local-profile picker
exposes a feature (multiple labelled save slots on one PC) that is
mostly noise.

**Root cause:** `currentProfilePtr` is a single `ProfileData*` and
`HexagonClient`'s login state is wholly orthogonal to it. The profile
JSON has no `steamId` or `loginName` field; the server stores by Steam
ID; nothing reconciles them.

**Backend changes needed:**

1. Introduce a small `IProfile` interface (or just a struct) with
   `getScore(levelId, difficulty)`, `setScore(...)`,
   `isFavorite(levelId)`, `setFavorite(levelId, bool)`.
2. Replace `currentProfilePtr` with a single offline profile (no more
   picker) and a side-by-side online overlay when logged in.
3. On login, optionally pull server scores for the Steam ID and
   merge: `local OR remote` (whichever is better) for scores;
   union for favorites; offer a one-time "I have multiple old
   profiles" import path that consolidates them.
4. Decouple favorites from `ProfileData` (favorites are a player-level
   concept, not a save-slot concept) -- though storing them on the
   single offline profile is fine if we keep one.

### Flaw F5 -- Level preview only shows colours, not patterns

**Symptom:** the menu shows a rotating coloured hexagon but no clue
whether the level is a slow pattern level, a fast bullet-hell, etc.

**Root cause:** the preview panel
([drawLevelSelectionLeftSide](../src/SSVOpenHexagon/Core/MenuGame.cpp#L5127-L5376))
shows only metadata (name/desc/diff/music/pack/best score). The
"preview background" is `StyleData::drawBackgroundMenu`
([MenuGame.cpp:5546](../src/SSVOpenHexagon/Core/MenuGame.cpp#L5546)),
which draws **a fixed 6-sided polygon coloured by the level's
StyleData**. It doesn't run the level's Lua, doesn't spawn walls,
doesn't render any pattern.

The gameplay engine *can* run a level headlessly --
`HexagonGame::runReplayUntilDeathAndGetScore` is used by the server to
validate replays -- so the infrastructure for "run a level without a
human" exists. There is no equivalent "render a few seconds of the
level into a render-texture for preview."

**Backend changes needed:**

1. A `HexagonGame::renderPreviewFrame(levelId, difficultyMult,
   timeOffsetSec) -> sf::Texture` (or live-running into a render
   texture).
2. Optionally cache the result so we don't re-run Lua every frame; a
   short looped render-texture (a few seconds of gameplay at a fixed
   seed) covers the use case.
3. Decide whether to run preview "from t=0" (consistent) or from a
   pseudo-random offset (more representative for long levels).

### Flaw F6 -- No search / filter / sort on the level selection screen

**Symptom:** the only way to find a level is to pack-jump (FOCUS+up/down)
and arrow through. With dozens of packs and hundreds of levels, this is
unworkable.

**Root cause:**

- `LevelDrawer` holds `levelDataIds: const Vector<String>*`
  ([MenuGame.hpp:451-452](../include/SSVOpenHexagon/Core/MenuGame.hpp#L451))
  -- a *non-owning pointer* into `HGAssets::levelDataIdsByPack`.
- That source vector is fixed at load time, sorted by `menuPriority`
  ([Assets.cpp:348-350](../src/SSVOpenHexagon/Global/Assets.cpp#L348-L350)).
- Pack ordering is fixed by `PackData::priority`
  ([Assets.cpp:357-359](../src/SSVOpenHexagon/Global/Assets.cpp#L357-L359)).
- `LevelData` carries no tags, no completion flags, no last-played
  timestamp, no estimated duration -- only `name`/`author`/`description`
  /`difficultyMults`. Nothing to filter on except those four.
- `ProfileData` has scores keyed by `levelValidator` (per-difficulty
  hash) but no per-level metadata (last-played, play count, completion
  flag).

**Backend changes needed:**

1. Add an owning, mutable level list inside the menu controller: a
   `Vector<LevelId>` plus current sort-key + filter-spec, recomputed
   when either changes.
2. Extend `LevelData` with `tags: Vector<String>` (parsed from
   `level.json`).
3. Extend `ProfileData` with per-level state: `last_played_ts`,
   `play_count`, `is_completed`. The existing `scores` map is keyed by
   level *validator* (per-difficulty); the extra state should be keyed
   by `levelId` (per-level, all difficulties).
4. A simple substring-match filter is enough for v1; fuzzy match
   (Levenshtein or trigram) is a v2 nice-to-have.
5. Repurpose the existing `drawEnteringText()` UI for the search input
   ([MenuGame.cpp:4016-4064](../src/SSVOpenHexagon/Core/MenuGame.cpp#L4016-L4064))
   -- the text-entry plumbing already exists, we just need a different
   target for the entered string.

### Flaw F7 -- No easy way to filter / sort levels

Same root cause as F6 -- covered above. To call out specifically what
sort keys are *easy* to add (data already present):

- By `LevelData::name` (alphabetical) -- trivial.
- By `LevelData::author` -- trivial.
- By min/max `LevelData::difficultyMults[]` -- trivial.
- By personal best score (`ProfileData::scores`) -- trivial.
- By "completed yes/no" -- needs the new `is_completed` bit (cheap to
  derive from `score > 0` but a real flag is cleaner).
- By "last played" -- needs a new `last_played_ts` per level.
- By "play count" -- needs a new `play_count` per level.

### Flaw F8 -- No easy way to favourite / unfavourite levels

**Surprise: the favourites infrastructure already exists.**
[`ProfileData::favoriteLevelDataIDs`](../include/SSVOpenHexagon/Data/ProfileData.hpp#L25)
persists per-profile to JSON. The level-selection screen has a
dedicated `favSlct` `LevelDrawer`
([MenuGame.hpp:451-452](../include/SSVOpenHexagon/Core/MenuGame.hpp#L451-L452))
and an "all levels ↔ favourites" toggle bound to F2.
`addRemoveFavoriteLevel()`
([MenuGame.cpp:4694-4785](../src/SSVOpenHexagon/Core/MenuGame.cpp#L4694-L4785))
is wired to F1. There's even an alphabetical sort of the favourites
list.

The flaw is purely **discoverability and UI**:

- F1/F2 hotkeys are only mentioned in a small "FAVORITE" / "UNFAVORITE"
  label on the right-side panel
  ([MenuGame.cpp:5300-5334](../src/SSVOpenHexagon/Core/MenuGame.cpp#L5300-L5334)).
- No star icon next to favourited levels in the main list.
- No "all + favourites pinned to top" view; it's an exclusive toggle
  between two lists.
- Mouse interaction is a deferred-action flag (`mustFavorite`), which
  works but is an unusual idiom.

The rewrite needs no backend changes for F8 -- just a clear UI.

---

## 6. Cross-cutting code smells

These don't map 1:1 to a user flaw but make every change harder.

1. **Five-thousand-line god class** -- `MenuGame.cpp` is the entire UI.
2. **OOP that adds no value** -- `ItemBase` hierarchy with five subclasses
   that exist only to encode "what does `exec()` do." A flat enum +
   data + a switch in the renderer would be smaller and clearer.
3. **Per-screen bespoke renderers** -- no widget abstraction, no layout
   primitives. Each new screen requires a fresh ~300-line
   `drawXxxScreen()` method with hand-tuned coordinates.
4. **State fragmentation** -- animation offsets live on `Category::offset`,
   `ItemBase::offset`, `LevelDrawer::YOffset`, plus a half-dozen
   screen-specific `Offset`/`Offset_packChange`/`Offset_details`
   variables. No single source of truth for "what's animating."
5. **Deferred-action mouse flags** -- `mustChangeIndexTo`, `mustFavorite`,
   `mustPlay`, `mustChangePackIndexTo`, `mustUseMenuItem` instead of an
   event queue. New mouse interactions add new flags.
6. **`*Action()` cascades** -- input dispatch is N parallel functions
   each with a big `switch(state)`. New screens add cases to all of them.
7. **Three view layers + manual coordinate conversion** -- every draw
   call has to choose `drawBackground`/`drawOverlay`/`drawScreen` and
   the implicit transform.
8. **Aspect-ratio special-casing** -- `fourByThree` boolean branches
   instead of a unit-aware layout system.
9. **No retained geometry** -- vertex buffers are `clear()`'d and
   re-filled every frame, even for screens whose content didn't change.
10. **Two copies of `SSVMenuSystem`** -- vendored fork in `include/` plus
    the unused upstream submodule in `extlibs/`.
11. **Hidden hotkeys** -- F1/F2 (favourites), FOCUS+arrows (pack jump)
    only discoverable by reading docs or the small status-bar text. The
    UI doesn't surface its own shortcuts.

---

## 7. Constraints the rewrite must respect

Even though the user asked for "from scratch," some things in the
ecosystem are not negotiable:

1. **VRSFML is the rendering layer.** Use immediate-mode
   `window.draw(font, sf::TextData{...})` and shape draw calls. No
   ImGui, no separate retained-mode widget tree.
2. **Lua scripts and existing pack format stay.** The level metadata
   schema can grow (add tags, etc.) but `pack.json`/`level.json` files
   shipped today by content creators must keep working -- workshop
   packs can't be retroactively migrated.
3. **The protocol stays compatible until everyone updates.** Adding
   new packet types is fine; renaming/removing old ones breaks running
   clients. Consider a protocol-version bump as part of the rewrite.
4. **Steam SDK availability is not guaranteed.** The game runs without
   Steam (offline mode). All Workshop/UGC entry points must
   gracefully degrade when `Steam::steam_manager` reports
   `Failed to initialize Steam API`.
5. **Headless / server mode must keep working.** `HexagonGame` is used
   server-side for replay validation; whatever happens in the menu
   layer must not pull the menu into the server build.

---

## 8. File index -- quick reference

UI:
- [`include/SSVOpenHexagon/Core/MenuGame.hpp`](../include/SSVOpenHexagon/Core/MenuGame.hpp) -- 639 lines, state enum + ~80 members
- [`src/SSVOpenHexagon/Core/MenuGame.cpp`](../src/SSVOpenHexagon/Core/MenuGame.cpp) -- 5,903 lines
- [`include/SSVOpenHexagon/Core/BindControl.hpp`](../include/SSVOpenHexagon/Core/BindControl.hpp) -- keyboard/joystick bind items
- [`src/SSVOpenHexagon/Core/BindControl.cpp`](../src/SSVOpenHexagon/Core/BindControl.cpp)
- [`include/SSVOpenHexagon/MenuSystem/`](../include/SSVOpenHexagon/MenuSystem/) -- vendored fork (used)
- [`extlibs/SSVMenuSystem/`](../extlibs/SSVMenuSystem/) -- upstream submodule (unused, can be removed)

Backend:
- [`src/SSVOpenHexagon/Core/Steam.cpp`](../src/SSVOpenHexagon/Core/Steam.cpp) /
  [`include/SSVOpenHexagon/Core/Steam.hpp`](../include/SSVOpenHexagon/Core/Steam.hpp)
- [`src/SSVOpenHexagon/Global/Assets.cpp`](../src/SSVOpenHexagon/Global/Assets.cpp) /
  [`include/SSVOpenHexagon/Global/Assets.hpp`](../include/SSVOpenHexagon/Global/Assets.hpp)
- [`include/SSVOpenHexagon/Data/LevelData.hpp`](../include/SSVOpenHexagon/Data/LevelData.hpp)
- [`include/SSVOpenHexagon/Data/PackData.hpp`](../include/SSVOpenHexagon/Data/PackData.hpp)
- [`include/SSVOpenHexagon/Data/ProfileData.hpp`](../include/SSVOpenHexagon/Data/ProfileData.hpp)
- [`include/SSVOpenHexagon/Online/Shared.hpp`](../include/SSVOpenHexagon/Online/Shared.hpp) -- packet types
- [`include/SSVOpenHexagon/Online/DatabaseRecords.hpp`](../include/SSVOpenHexagon/Online/DatabaseRecords.hpp) -- User schema
- [`src/SSVOpenHexagon/Online/Database.cpp`](../src/SSVOpenHexagon/Online/Database.cpp)
- [`src/SSVOpenHexagon/Online/Sodium.cpp`](../src/SSVOpenHexagon/Online/Sodium.cpp) -- current (weak) password hash
- [`src/SSVOpenHexagon/Core/HexagonClient.cpp`](../src/SSVOpenHexagon/Core/HexagonClient.cpp)
- [`src/SSVOpenHexagon/Core/HexagonServer.cpp`](../src/SSVOpenHexagon/Core/HexagonServer.cpp)
- [`src/OHWorkshopUploader/main.cpp`](../src/OHWorkshopUploader/main.cpp) -- content-creator CLI; not in-game

---

*End of research document. Next step: design proposal.*

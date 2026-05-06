// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Forward declarations of every screen draw function. Each lives in its own
// translation unit under `src/SSVOpenHexagon/UI/Screens/`. A single dispatcher
// (`drawCurrentScreen`) calls the right one based on `App::current`.

#include "SSVOpenHexagon/UI/App.hpp" // for `Screen`

namespace hg::ui
{

struct Context;
struct Services;

void drawCurrentScreen(Context& ctx, App& app, Services& svc);

// Per-screen draw functions. Add new declarations here as screens are added.
void drawMainScreen           (Context& ctx, App& app, Services& svc);
void drawOptionsScreen        (Context& ctx, App& app, Services& svc);
void drawLevelSelectScreen    (Context& ctx, App& app, Services& svc);
void drawWorkshopBrowseScreen (Context& ctx, App& app, Services& svc);
void drawOnlineScreen         (Context& ctx, App& app, Services& svc);

// Navigation helpers used by every screen. Implementation lives next to the
// dispatcher.
void pushScreen(App& app, Screen target); //!< saves current on backStack, transitions to target
void goBack    (App& app);                //!< pops backStack (no-op if empty)

// Convenience: when `ctx.input.escape` is set, pop the screen and return
// true -- caller should immediately `return` to skip the rest of the draw.
// Replaces the `if (ctx.input.escape) { goBack(app); return; }` block at
// the top of every screen.
[[nodiscard]] bool handleEscape(const Context& ctx, App& app);

} // namespace hg::ui

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include <cstdio>

namespace hg::ui
{

namespace
{

constexpr int kBackItemIdx = 0;

void drawStatRow(Context& ctx, const char* labelText, const char* value)
{
    label(ctx, labelText);
    // Cheap right-aligned-ish: re-position cursor and draw value at offset.
    ctx.cursor.y -= ctx.rowHeight; // back to the same row
    ctx.cursor.x += 200.f;
    label(ctx, value);
}

} // namespace

void drawProfileScreen(Context& ctx, App& app, Services& /*svc*/)
{
    ProfileScreenState& s = app.profile;

    // Navigation: only one interactive item for now (Back), so up/down do
    // nothing meaningful; escape always pops.
    if (ctx.input.escape)
    {
        goBack(app);
        return;
    }

    stepToward(s.openProgress, 1.f, ctx.dt, 6.f);
    stepToward(s.selectionY, static_cast<float>(s.selectedIdx) * ctx.rowHeight, ctx.dt, 18.f);

    const sf::Vec2u sz = ctx.target->getSize();
    ctx.cursor = ctx.origin = {static_cast<float>(sz.x) * 0.5f - 200.f, static_cast<float>(sz.y) * 0.20f};

    heading(ctx, "PROFILE");

    // ---- Read-only summary --------------------------------------------------
    {
        char nameLine[128] = {};
        std::snprintf(nameLine, sizeof(nameLine), "NAME:  %s", app.profileSnapshot.name);
        label(ctx, nameLine);
    }

    {
        char scoredLine[64] = {};
        std::snprintf(scoredLine, sizeof(scoredLine), "LEVELS PLAYED:  %d", app.profileSnapshot.totalScored);
        label(ctx, scoredLine);
    }

    {
        char favLine[64] = {};
        std::snprintf(favLine, sizeof(favLine), "FAVORITES:  %d", app.profileSnapshot.totalFavorites);
        label(ctx, favLine);
    }

    {
        char onlineLine[128] = {};
        std::snprintf(onlineLine, sizeof(onlineLine), "ONLINE:  %s", app.profileSnapshot.onlineStatus);
        label(ctx, onlineLine);
    }

    newLine(ctx, 12.f);
    separator(ctx);
    newLine(ctx, 4.f);

    // ---- Back button --------------------------------------------------------
    if (button(ctx, "BACK", s.selectedIdx == kBackItemIdx))
    {
        goBack(app);
        return;
    }
}

} // namespace hg::ui

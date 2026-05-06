// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include <algorithm>

namespace hg::ui
{

namespace
{

// The new main menu has six entries; "WORKSHOP" is added vs. the old menu and
// "LOCAL PROFILES" becomes plain "PROFILE" (single-profile model — see the
// design doc, F4). Items that route into the new UI use `pushScreen`; items
// that route into still-old screens go through `Services` callbacks.
constexpr struct
{
    const char* label;
    void (*activate)(App&, Services&);
} kItems[] = {
    {"PLAY",     [](App& a, Services&)      { pushScreen(a, Screen::LevelSelect); }},
    {"WORKSHOP", [](App&, Services& s)      { if (s.onWorkshopRequested) s.onWorkshopRequested(); }},
    {"OPTIONS",  [](App& a, Services&)      { pushScreen(a, Screen::Options); }},
    {"PROFILE",  [](App& a, Services&)      { pushScreen(a, Screen::Profile); }},
    {"ONLINE",   [](App&, Services& s)      { if (s.onOnlineRequested)   s.onOnlineRequested(); }},
    {"EXIT",     [](App& a, Services& s)    { a.exitRequested = true; if (s.onExit) s.onExit(); }},
};

constexpr int kItemCount = static_cast<int>(sizeof(kItems) / sizeof(kItems[0]));

} // namespace

void drawMainScreen(Context& ctx, App& app, Services& svc)
{
    MainScreenState& s = app.main;

    // ---- Input -------------------------------------------------------------
    if (ctx.input.up)
    {
        s.selectedIdx = (s.selectedIdx + kItemCount - 1) % kItemCount;
        if (svc.playSound) svc.playSound("beep.ogg");
    }
    if (ctx.input.down)
    {
        s.selectedIdx = (s.selectedIdx + 1) % kItemCount;
        if (svc.playSound) svc.playSound("beep.ogg");
    }

    // ---- Animation step ----------------------------------------------------
    // `selectionY` lerps toward the focused row's vertical position. The
    // visual selection cursor is drawn at `selectionY`, not at `selectedIdx`.
    stepToward(s.selectionY, static_cast<float>(s.selectedIdx) * ctx.rowHeight, ctx.dt, 18.f);

    // `openProgress` ramps 0 -> 1 once when this screen is entered. Reset
    // upstream when the screen changes (see App-level transition handling).
    stepToward(s.openProgress, 1.f, ctx.dt, 6.f);

    // ---- Layout ------------------------------------------------------------
    const sf::Vec2u sz   = ctx.target->getSize();
    const float     col  = static_cast<float>(sz.x) * 0.5f - 200.f;
    const float     top  = static_cast<float>(sz.y) * 0.30f;

    ctx.cursor = ctx.origin = {col, top};
    heading(ctx, "OPEN HEXAGON");

    // Animated selection pill behind the focused item, drawn before the rows
    // so the rows render on top.
    {
        const sf::Vec2f pillPos{ctx.cursor.x - 8.f, ctx.cursor.y + s.selectionY - 4.f};
        const sf::Vec2f pillSize{360.f + 16.f, ctx.rowHeight + 8.f};
        ctx.target->draw(sf::RectangleShapeData{
            .position  = pillPos,
            .fillColor = ctx.colAccent,
            .size      = pillSize,
        });
    }

    // ---- Items -------------------------------------------------------------
    for (int i = 0; i < kItemCount; ++i)
    {
        // Per-item staggered fade based on `openProgress`.
        const float itemAlpha = easeOutCubic(
            std::min(1.f, std::max(0.f, s.openProgress * static_cast<float>(kItemCount + 2) - static_cast<float>(i))));

        // Apply alpha by tinting the text colour. We don't push/pop colour
        // (no theme stack); we just compute the right value here.
        const sf::Color textBefore     = ctx.colText;
        const sf::Color highlightBefore = ctx.colHighlight;
        ctx.colText      = sf::Color{textBefore.r,      textBefore.g,      textBefore.b,
                                     static_cast<unsigned char>(textBefore.a * itemAlpha)};
        ctx.colHighlight = sf::Color{highlightBefore.r, highlightBefore.g, highlightBefore.b,
                                     static_cast<unsigned char>(highlightBefore.a * itemAlpha)};

        const bool focused = (i == s.selectedIdx);
        if (button(ctx, kItems[i].label, focused))
        {
            kItems[i].activate(app, svc);
            // Don't keep iterating after an action — the screen may have
            // changed underneath us.
            ctx.colText      = textBefore;
            ctx.colHighlight = highlightBefore;
            return;
        }

        ctx.colText      = textBefore;
        ctx.colHighlight = highlightBefore;
    }
}

} // namespace hg::ui

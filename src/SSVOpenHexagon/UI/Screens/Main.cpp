// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/MinMax.hpp"

namespace hg::ui
{

namespace
{

// The new main menu has six entries; "WORKSHOP" is added vs. the old menu and
// "LOCAL PROFILES" becomes plain "PROFILE" (single-profile model -- see the
// design doc, F4). Items that route into the new UI use `pushScreen`; items
// that route into still-old screens go through `Services` callbacks.
constexpr struct
{
    const char* label;
    void (*activate)(App&, Services&);
} kItems[] = {
    {"PLAY", [](App& a, Services&) { pushScreen(a, Screen::LevelSelect); }},
    {"WORKSHOP", [](App& a, Services&) { pushScreen(a, Screen::WorkshopBrowse); }},
    {"OPTIONS", [](App& a, Services&) { pushScreen(a, Screen::Options); }},
    {"ONLINE", [](App& a, Services&) { pushScreen(a, Screen::Online); }},
    {"EXIT",
     [](App& a, Services& s)
{
    a.exitRequested = true;
    if (s.onExit)
        s.onExit();
}},
};

constexpr int kItemCount = static_cast<int>(sizeof(kItems) / sizeof(kItems[0]));

} // namespace

void drawMainScreen(Context& ctx, App& app, Services& svc)
{
    const ScopedTransform tg{ctx, sf::Transform{}.scaleBy({1.35f, 1.35f})};

    MainScreenState& s = app.main;

    navigateList(ctx, svc, s.selectedIdx, kItemCount);
    stepToward(s.selectionY, static_cast<float>(s.selectedIdx) * ctx.rowHeight, ctx.dt, 256.f);
    stepToward(s.openProgress, 1.f, ctx.dt, 6.f);

    // Custom title block -- replaces the legacy `titleBar.png` logo. Two
    // stacked outlined words ("OPEN" then a much bigger "HEXAGON") with
    // the game version pinned to the right of the upper line. The accent
    // outline is the gradient sentinel, so the post-process shader paints
    // an animated gradient through every glyph border.
    ctx.cursor = ctx.origin = screenOrigin(ctx);

    constexpr float kOpenSize    = 56.f;
    constexpr float kHexSize     = 110.f;
    constexpr float kVersionSize = 24.f;

    // Real glyph bounds -- no per-character-advance guessing. The bounds'
    // `position` carries the natural top-left offset SFML applies to a
    // glyph string; we shift draw positions by `-bounds.position` so the
    // visible rect starts exactly where we ask it to.
    const sf::Rect2f openBounds = measureText(ctx, "OPEN", kOpenSize);
    const sf::Rect2f hexBounds  = measureText(ctx, "HEXAGON", kHexSize);
    const sf::Rect2f verBounds  = measureText(ctx, GAME_VERSION_STR_V, kVersionSize);

    constexpr float kPad     = 12.f;
    constexpr float kVerGap  = 24.f; // horizontal gap "OPEN" → version
    constexpr float kLineGap = 4.f;  // vertical breathing room between lines

    const float lineWidth   = sf::base::max(openBounds.size.x + kVerGap + verBounds.size.x, hexBounds.size.x);
    const float blockWidth  = lineWidth + kPad * 2.f;
    const float blockHeight = openBounds.size.y + kLineGap + hexBounds.size.y + kPad * 2.f;

    // Solid backdrop behind the title -- the gradient outlines pop off
    // it and it hides any background-level visuals that would otherwise
    // show through the gaps between glyphs.
    ctx.target->draw(
        sf::RectangleShapeData{
            .position  = ctx.cursor,
            .fillColor = ctx.colRow,
            .size      = {blockWidth, blockHeight},
        },
        ctx.renderStates);

    // Inner content cursor -- top-left of the actual text area.
    const sf::Vec2f content{ctx.cursor.x + kPad, ctx.cursor.y + kPad};

    // First line: "OPEN" left-aligned, "2.2.0" centered vertically next to it.
    text(ctx, {content.x - openBounds.position.x, content.y - openBounds.position.y}, "OPEN", kOpenSize);

    // Second line: "HEXAGON" left-aligned beneath.
    text(ctx,
         {content.x - hexBounds.position.x, content.y + openBounds.size.y + kLineGap - hexBounds.position.y},
         "HEXAGON",
         kHexSize);

    // Version overlay: drawn on top of "HEXAGON", right-aligned to the
    // title block, vertically centered against the HEXAGON line.
    text(ctx,
         {content.x + lineWidth - verBounds.size.x - verBounds.position.x,
          content.y + openBounds.size.y - verBounds.size.y - verBounds.position.y},
         GAME_VERSION_STR_V,
         kVersionSize);

    ctx.cursor.y += blockHeight + 24.f;

    pill(ctx, {ctx.cursor.x, ctx.cursor.y + s.selectionY}, 360.f);

    // Per-item staggered fade-in driven by `openProgress`. We tint the
    // text colors temporarily, draw the row, then restore -- saves a
    // theme-stack abstraction we'd only use in this one place.
    const sf::Color textBefore      = ctx.colText;
    const sf::Color highlightBefore = ctx.colHighlight;
    const auto      tintAlpha       = [](sf::Color c, float a)
    { return sf::Color{c.r, c.g, c.b, static_cast<unsigned char>(c.a * a)}; };

    for (int i = 0; i < kItemCount; ++i)
    {
        const float itemAlpha = easeOutCubic(
            sf::base::clamp(s.openProgress * static_cast<float>(kItemCount + 2) - static_cast<float>(i), 0.f, 1.f));

        ctx.colText      = tintAlpha(textBefore, itemAlpha);
        ctx.colHighlight = tintAlpha(highlightBefore, itemAlpha);

        if (button(ctx, kItems[i].label, i == s.selectedIdx))
        {
            ctx.colText      = textBefore;
            ctx.colHighlight = highlightBefore;
            kItems[i].activate(app, svc);
            return; // screen may have changed under us
        }
    }

    ctx.colText      = textBefore;
    ctx.colHighlight = highlightBefore;
}

} // namespace hg::ui

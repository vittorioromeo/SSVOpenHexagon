// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Transform.hpp"

#include <algorithm>

namespace hg::ui
{

void pushScreen(App& app, Screen target)
{
    app.previousScreen = app.current;
    app.backStack.emplaceBack(app.current);
    app.current = target;
}

void goBack(App& app)
{
    if (app.backStack.empty())
    {
        return;
    }
    app.previousScreen = app.current;
    app.current        = app.backStack.back();
    app.backStack.popBack();
}

bool handleEscape(const Context& ctx, App& app)
{
    if (!ctx.input.escape)
        return false;
    goBack(app);
    return true;
}

namespace
{

// Visual depth of sub-screens above the Main menu. When a sub-screen is
// active, Main slides this many pixels left (partially out of the
// viewport) so the user still sees it as the "back" of the stack.
constexpr float kSubscreenSlideX = 1024.f;

// How much of the original alpha the backgrounded Main keeps. 1.0 = full,
// 0.0 = invisible. ~30% reads as "muted but still there".
constexpr float kSubscreenFadeMul = 0.30f;

// Total real-time duration of one push/pop transition, in seconds.
// `backDepthAnim` advances linearly toward its target at `1 / kTransitionSeconds`
// per frame; easing is applied at render time.
constexpr float kTransitionSeconds = 0.05f;

// Threshold at which a screen counts as "fully settled" and starts
// accepting input again. Anything during the transition is locked out.
constexpr float kInputIdleEpsilon = 0.05f;

void dispatchToScreen(Context& ctx, App& app, Services& svc, Screen which)
{
    switch (which)
    {
        case Screen::Main:
            drawMainScreen(ctx, app, svc);
            break;
        case Screen::Options:
            drawOptionsScreen(ctx, app, svc);
            break;
        case Screen::LevelSelect:
            drawLevelSelectScreen(ctx, app, svc);
            break;
        case Screen::WorkshopBrowse:
            drawWorkshopBrowseScreen(ctx, app, svc);
            break;
        case Screen::Online:
            drawOnlineScreen(ctx, app, svc);
            break;
    }
}

// Render a screen with a per-frame slide offset, a global alpha multiplier,
// and an optional input-suppression flag -- saving and restoring every
// `Context` field we mutate so the outer dispatcher logic is unaffected.
void drawScreenWithEffects(Context& ctx, App& app, Services& svc, Screen which, float slideX, float alpha, bool acceptInput)
{
    if (alpha <= 0.001f)
        return; // fully transparent -- skip the work entirely

    const sf::Transform savedTransform   = ctx.renderStates.transform;
    const Input         savedInput       = ctx.input;
    const sf::Color     savedText        = ctx.colText;
    const sf::Color     savedHighlight   = ctx.colHighlight;
    const sf::Color     savedTextDim     = ctx.colTextDim;
    const sf::Color     savedAccent      = ctx.colAccent;
    const sf::Color     savedRow         = ctx.colRow;
    const sf::Color     savedRowFocused  = ctx.colRowFocused;
    const float         savedScreenAlpha = ctx.screenAlpha;

    if (slideX != 0.f)
    {
        ctx.renderStates.transform = savedTransform * sf::Transform::fromPosition({slideX, 0.f});
    }
    // Surface the per-screen alpha so screens that draw custom sprites or
    // hardcoded colors can fade in lockstep with the standard widgets
    // (which fade automatically because we mutate `colText`/`colRow`/etc.).
    ctx.screenAlpha = savedScreenAlpha * alpha;
    if (alpha < 0.999f)
    {
        const auto fade = [alpha](sf::Color c)
        { return sf::Color{c.r, c.g, c.b, static_cast<sf::base::U8>(c.a * alpha)}; };
        ctx.colText       = fade(savedText);
        ctx.colHighlight  = fade(savedHighlight);
        ctx.colTextDim    = fade(savedTextDim);
        ctx.colAccent     = fade(savedAccent);
        ctx.colRow        = fade(savedRow);
        ctx.colRowFocused = fade(savedRowFocused);
    }
    if (!acceptInput)
        ctx.input = {};

    dispatchToScreen(ctx, app, svc, which);

    ctx.renderStates.transform = savedTransform;
    ctx.input                  = savedInput;
    ctx.colText                = savedText;
    ctx.colHighlight           = savedHighlight;
    ctx.colTextDim             = savedTextDim;
    ctx.colAccent              = savedAccent;
    ctx.colRow                 = savedRow;
    ctx.colRowFocused          = savedRowFocused;
    ctx.screenAlpha            = savedScreenAlpha;
}

} // namespace

// `backDepthAnim` is a single 0..1 timeline:
//   0 = Main menu centered, full alpha; no sub-screen visible.
//   1 = Main menu slid + faded as a backdrop; sub-screen at full alpha.
//
// Push (Main → Sub) drives it 0 → 1; pop (Sub → Main) drives it 1 → 0.
// Main's slide+fade and the sub-screen's fade run simultaneously over the
// full `t` range, each with its own easing -- symmetric on push and pop.
void drawCurrentScreen(Context& ctx, App& app, Services& svc)
{
    // Linear time-based advance. Easing is applied to the rendered values
    // below, so push and pop share a single update path.
    const float target = (app.current == Screen::Main) ? 0.f : 1.f;
    const float step   = ctx.dt / kTransitionSeconds;
    if (app.backDepthAnim < target)
        app.backDepthAnim = std::min(target, app.backDepthAnim + step);
    else if (app.backDepthAnim > target)
        app.backDepthAnim = std::max(target, app.backDepthAnim - step);

    const float t = app.backDepthAnim;

    // Main slides + fades over the full [0, 1] timeline. easeOutCubic
    // gives a snappy start that decelerates into the backdrop position.
    const float exitT        = easeOutCubic(t);
    const float mainSlideX   = -kSubscreenSlideX * exitT;
    const float mainAlphaMul = 1.f - exitT * (1.f - kSubscreenFadeMul);

    // Sub-screen fades in concurrently. easeInOutCubic gives a soft-soft
    // curve so it doesn't pop in or out. It also slides in from the right
    // so the preview / metadata travel with the rest of the UI rather
    // than appearing in place -- matches the "stack" feel of Main sliding
    // out left.
    const float subAlpha  = easeInOutCubic(t);
    const float subSlideX = kSubscreenSlideX * (1.f - exitT);

    // The sub-screen we render: when the user is on a sub-screen, that's
    // `current`. When they just popped back to Main, it's `previousScreen`
    // -- needed so the outgoing screen can fade out smoothly.
    const Screen subScreen = (app.current != Screen::Main) ? app.current : app.previousScreen;
    const bool   hasSub    = (subScreen != Screen::Main) && (t > 0.001f);

    // Main: always rendered. Slide / alpha are 0 / 1 at idle (t=0), grow
    // toward backdrop values as `t` rises. Input is gated on Main being
    // the current screen AND the transition having settled.
    const bool mainIdle = (app.current == Screen::Main) && (t < kInputIdleEpsilon);
    drawScreenWithEffects(ctx, app, svc, Screen::Main, mainSlideX, mainAlphaMul, mainIdle);

    // Sub-screen: only rendered while transitioning or fully visible.
    // Input is gated on it being the current screen AND the transition
    // having settled.
    if (hasSub)
    {
        const bool subIdle = (app.current == subScreen) && (t > 1.f - kInputIdleEpsilon);
        drawScreenWithEffects(ctx,
                              app,
                              svc,
                              subScreen,
                              subSlideX,
                              subAlpha,
                              subIdle);
    }
}

} // namespace hg::ui

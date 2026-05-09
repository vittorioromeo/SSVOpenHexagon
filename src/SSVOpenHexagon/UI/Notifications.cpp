// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Notifications.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"


namespace hg::ui
{

namespace
{

// Anchor-padding from the bottom-right corner of the viewport.
constexpr float kMarginRight  = 16.f;
constexpr float kMarginBottom = 16.f;

// Spacing between stacked toasts.
constexpr float kStackGap = 8.f;

// Toast geometry.
constexpr float kFrameSize = 4.f;   //!< outer (magenta) frame thickness
constexpr float kInnerPadX = 14.f;  //!< horizontal text padding inside backdrop
constexpr float kInnerPadY = 10.f;  //!< vertical text padding inside backdrop
constexpr float kFontSize  = 18.f;  //!< matches the modal "press any key" line size
constexpr float kMaxWidth  = 380.f; //!< clamp before truncation; ample for short messages
constexpr float kMinWidth  = 220.f; //!< below this looks lonely

// Animation envelope: 220ms slide-in from the right, hold, 320ms slide-out.
// We deliberately do NOT fade alpha because the menu's accent-gradient
// shader keys on magenta saturation (`min(R,B) - G > threshold`) -- when
// we draw an alpha-faded magenta frame into the cleared composite the
// rendered RGB drops proportionally and the shader's threshold cuts
// out, swapping the gradient frame for plain dim magenta partway
// through the fade. Sliding the toast off-screen instead avoids that
// artifact entirely: the toast renders at full alpha for its whole
// lifetime and disappears from view by translation alone, in sync with
// `tickNotifications` removing it.
constexpr float kSlideInSec  = 0.22f;
constexpr float kSlideOutSec = 0.32f;

// Per-glyph width fraction used to size the toast around its text.
// Must match (or slightly exceed) `truncateToFit`'s assumption in
// [UI.cpp:766]: `charW = fontSize * 0.6f`. We pad above that so the
// computed toast width always leaves room for the full string and
// `truncateToFit` never trims it.
constexpr float kCharAdvance = 0.65f;

// Extra horizontal slack on top of the per-glyph estimate, in pixels.
// Compensates for outline thickness and any glyph wider than the
// monospace approximation.
constexpr float kWidthSafetyPad = 12.f;

[[nodiscard]] float toastWidth(const Notification& n) noexcept
{
    const float estimated = static_cast<float>(n.text.size()) * (kFontSize * kCharAdvance) //
                            + 2.f * kInnerPadX                                             //
                            + kWidthSafetyPad;
    return sf::base::clamp(estimated, kMinWidth, kMaxWidth);
}

[[nodiscard]] float toastHeight() noexcept
{
    return kFontSize + 2.f * kInnerPadY;
}

// Returns the horizontal offset (positive = pushes the toast to the right
// / off-screen) for the current animation phase. During slide-in, eases
// from "fully off-screen right" to the toast's resting position. During
// hold, returns 0. During slide-out, eases back to fully off-screen.
//
// `outOfViewPx` is the distance the toast must travel to be hidden by
// the right edge of the viewport: its width plus the right margin plus
// a small safety margin so subpixel rounding doesn't leave a sliver.
[[nodiscard]] float slideOffset(const Notification& n, float outOfViewPx) noexcept
{
    if (n.age < kSlideInSec)
    {
        const float t     = n.age / kSlideInSec;
        const float eased = 1.f - (1.f - t) * (1.f - t) * (1.f - t); // easeOutCubic
        return (1.f - eased) * outOfViewPx;
    }

    const float slideOutStart = n.lifetime - kSlideOutSec;
    if (n.age >= slideOutStart)
    {
        const float t     = (n.age - slideOutStart) / kSlideOutSec;
        const float clamp = sf::base::clamp(t, 0.f, 1.f);
        const float eased = clamp * clamp * clamp; // easeInCubic (slow start, fast finish)
        return eased * outOfViewPx;
    }

    return 0.f;
}

void drawRect(Context& ctx, sf::Vec2f pos, sf::Vec2f size, sf::Color fill)
{
    ctx.target->draw(
        sf::RectangleShapeData{
            .position  = pos,
            .fillColor = fill,
            .size      = size,
        },
        ctx.renderStates);
}

} // namespace

void pushNotification(NotificationStack& stack, sf::base::StringView text, float lifetime)
{
    stack.items.emplaceBack(Notification{.text = sf::base::String{text}, .age = 0.f, .lifetime = lifetime});
}

void tickNotifications(NotificationStack& stack, float dt)
{
    for (Notification& n : stack.items)
        n.age += dt;

    // Single-pass removal of expired toasts.
    sf::base::vectorEraseIf(stack.items, [](const Notification& n) { return n.age >= n.lifetime; });
}

void drawNotifications(Context& ctx, const NotificationStack& stack)
{
    if (ctx.target == nullptr || ctx.font == nullptr || stack.items.empty())
        return;

    // The host installs a global scale on `ctx.renderStates.transform`
    // (e.g. `sf::Transform{}.scaleBy({0.75f, 0.75f})` in
    // `MenuGame::drawNewMainMenu`). That makes screen layout easier
    // but also means a draw at the bottom-right of the viewport lands
    // at 75% of the way to the target edge, not the corner. Toasts
    // anchor to the actual target corner, so we save the host transform,
    // reset to identity for our draw, and restore on exit. The view
    // stays as-is so view-coords map 1:1 to target pixels here.
    const sf::Transform savedTransform = ctx.renderStates.transform;
    ctx.renderStates.transform         = sf::Transform{};
    SFML_BASE_SCOPE_GUARD({ ctx.renderStates.transform = savedTransform; });

    const sf::Vec2f vp = viewportSize(ctx);
    if (vp.x <= 0.f || vp.y <= 0.f)
        return;

    // The frame is the magenta-sentinel that the post-process gradient
    // shader recolors -- same trick the modal uses for its border, see
    // `HexagonDialogBox::drawBox`. The backdrop is opaque-ish black so
    // the gradient frame pops off it.
    const sf::Color frameColor    = ctx.colAccent;
    const sf::Color backdropColor = sf::Color{0, 0, 0, 220};

    // Draw oldest-first from the bottom up so the newest is closest to
    // the corner. (`vectorEraseIf` removes from the front-end of the
    // vector when it expires, so iterating back-to-front gives newest-first.
    // We want newest at the bottom; older entries float upward as their
    // older neighbours expire below them.)
    float yCursor = vp.y - kMarginBottom;
    for (sf::base::SizeT i = stack.items.size(); i-- > 0;)
    {
        const Notification& n = stack.items[i];

        const float w = toastWidth(n);
        const float h = toastHeight();

        // Distance the toast must travel rightward to be fully past the
        // viewport's right edge. `+kMarginRight` so it's not just at the
        // edge but cleanly off-screen, +1 for subpixel safety.
        const float outOfViewPx = w + kMarginRight + 1.f;

        const float slide = slideOffset(n, outOfViewPx);

        const sf::Vec2f outerPos{vp.x - kMarginRight - w + slide, yCursor - h};
        const sf::Vec2f outerSize{w, h};

        // Frame (magenta-sentinel; gets gradient-shaded post-composite).
        // Drawn at full opacity intentionally -- see `kSlideInSec` doc.
        drawRect(ctx, outerPos, outerSize, frameColor);

        // Inner backdrop, inset by `kFrameSize` on every side.
        const sf::Vec2f innerPos{outerPos.x + kFrameSize, outerPos.y + kFrameSize};
        const sf::Vec2f innerSize{outerSize.x - 2.f * kFrameSize, outerSize.y - 2.f * kFrameSize};
        drawRect(ctx, innerPos, innerSize, backdropColor);

        // Vertically center the text inside the inner box. `kInnerPadY * 0.5`
        // lands the cap-height baseline close to the visual middle for
        // OpenSquare at `kFontSize`. `truncateToFit` (called inside `text`)
        // clamps anything that overshoots `textMaxW`, but the toast width
        // estimation in `toastWidth` adds enough headroom that it usually
        // shouldn't trigger.
        const sf::Vec2f textPos{innerPos.x + kInnerPadX, innerPos.y + kInnerPadY * 0.5f};
        const float     textMaxW = innerSize.x - 2.f * kInnerPadX;
        text(ctx, textPos, n.text.cStr(), kFontSize, ctx.colHighlight, textMaxW);

        yCursor = outerPos.y - kStackGap;
    }
}

} // namespace hg::ui

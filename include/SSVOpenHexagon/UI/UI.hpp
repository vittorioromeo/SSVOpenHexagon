// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Immediate-mode UI primitives for SSVOpenHexagon's new menu system.
// See `docs/UI_REWRITE_DESIGN.md` for design rationale.
//
// Conventions:
//   - Free functions, no widget classes.
//   - All drawing is immediate: every widget calls `target.draw(...)` with
//     a fresh `sf::TextData` / `sf::RectangleShapeData`. No retained geometry.
//   - State lives on the caller's `App`. Widgets never own state.
//   - Focus is per-screen: caller passes `focused` to each widget.
//   - Layout is a cursor + a few helpers; no layout solver.
//   - Static labels are passed as `const char*` (null-terminated literals).
//     Dynamic text uses `sf::base::String&`.

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderStates.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/StringView.hpp"

namespace sf
{
class Font;
class RenderTarget;
} // namespace sf

namespace sf::base
{
class String;
} // namespace sf::base

namespace hg::ui
{

struct Services;

////////////////////////////////////////////////////////////////////////////////
// Per-frame snapshot of input the UI cares about. Filled once at the top of
// the menu update; widgets read it but never poll SFML directly.

struct Input
{
    sf::Vec2f mousePos{};

    bool mousePressed{}; //!< edge: mouse button just pressed this frame
    bool mouseDown{};    //!< level: mouse button held this frame

    bool up{};        //!< edge: navigate up
    bool down{};      //!< edge: navigate down
    bool left{};      //!< edge: decrease / move left
    bool right{};     //!< edge: increase / move right
    bool enter{};     //!< edge: activate selection
    bool escape{};    //!< edge: back / cancel
    bool backspace{}; //!< edge: erase one character (text fields)

    // Null-terminated chars typed this frame (max 7 chars + NUL). Caller fills.
    char typedChars[8] = {0};
};

////////////////////////////////////////////////////////////////////////////////
// Drawing + layout state. Lives on the caller (one per app), passed by
// reference to every widget. No allocations.

struct Context
{
    sf::RenderTarget* target{};
    const sf::Font*   font{};
    Input             input{};

    // Render states applied to every primitive the UI draws this frame. Lets
    // the host (`MenuGame`) push a custom view / transform across all
    // widgets in one place. VRSFML doesn't auto-apply the target's view, so
    // every `target->draw(...)` call inside the UI passes this through.
    sf::RenderStates renderStates{};

    // Optional pointer to the host's `Services`. Widgets use it for sound
    // feedback (`playSound`) without having to thread `Services&` through
    // every call. Nullable for unit tests / non-game contexts.
    Services* services{};

    // Time delta of the current frame, in seconds. Animation helpers read this.
    float dt{1.f / 60.f};

    // Layout cursor. Widgets advance it as they draw. `origin` is what
    // `newColumn()` / `newLine()` reset cursor.x to.
    sf::Vec2f cursor{};
    sf::Vec2f origin{};

    // Default row height in screen-space pixels. Widgets respect this when
    // advancing the cursor.
    float rowHeight{34.f};

    // Theme. Hard-coded white-on-black with a magenta accent. The accent is
    // a sentinel color: the shader pass that composites the UI onto the
    // window replaces every magenta pixel with a screen-space gradient, so
    // pills and text outlines animate as a colored gradient at runtime.
    sf::Color colText{255, 255, 255, 255};
    sf::Color colTextDim{255, 255, 255, 160};
    sf::Color colHighlight{255, 255, 255, 255};
    sf::Color colAccent{255, 0, 255, 255};
    sf::Color colRow{0, 0, 0, 200};
    // Focused rows reuse the same dark transparent background as non-focused
    // rows so a focus pill drawn behind them shows through uniformly. Focus
    // is signaled by the pill underneath plus the highlighted text color.
    sf::Color colRowFocused{0, 0, 0, 200};

    float fontSize{22.f};

    // Cached "visual center" of glyph rendering at `fontSize`, computed
    // by `recomputeTextMetrics()` from `precomputeTextLocalBounds`. Used
    // by row widgets (button/toggle/label/slider/textField) to vertically
    // center their text in a row of arbitrary height. Without this we
    // get the eyeballed `(r.size.y - fontSize) * 0.5f` look, which only
    // matches by accident — actual glyph bounds depend on the font.
    float textCenterOffsetY{0.f};

    // Per-screen alpha multiplier set by the dispatcher during transitions
    // (1.0 when the screen is fully visible, 0.0 when invisible). The
    // standard widgets fade automatically because the dispatcher mutates
    // `colText` / `colRow` / etc. in lockstep; screens that draw custom
    // sprites or hardcoded colors must multiply by this to fade together.
    float screenAlpha{1.f};
};

////////////////////////////////////////////////////////////////////////////////
// Animation helpers. Plain floats, exponential ease toward target.

// Maps a window-pixel position into the UI's layout (model) space, using
// the view + transform stored on `ctx.renderStates`. The host should call
// this when filling `ctx.input.mousePos` so widget hit-testing matches the
// transformed render.
//
//   ctx.input.mousePos = hg::ui::screenToUI(
//       ctx, sf::Mouse::getPosition(window).to<sf::Vec2f>());
[[nodiscard]] sf::Vec2f screenToUI(const Context& ctx, sf::Vec2f pixelPos) noexcept;

// Returns true if a step actually moved `current` (i.e. animation not yet at
// target). `speed` is an inverse time constant; 12 ≈ "reach target in ~80 ms".
bool stepToward(float& current, float target, float dt, float speed = 12.f);
bool stepToward(sf::Vec2f& current, sf::Vec2f target, float dt, float speed = 12.f);

// Easing functions, t in [0, 1] -> [0, 1].
float easeOutCubic(float t) noexcept;
float easeInOutCubic(float t) noexcept;
float easeOutBack(float t) noexcept;

////////////////////////////////////////////////////////////////////////////////
// Layout helpers.

void newLine(Context& ctx, float dyExtra = 0.f);
void newColumn(Context& ctx, float dx);
void indent(Context& ctx, float dx);

// Rectangle of the current row at the cursor, given a width.
sf::Rect2f rowRect(const Context& ctx, float width) noexcept;

////////////////////////////////////////////////////////////////////////////////
// Display-only widgets.

// Display widgets paint a `colRow` row backdrop (semi-transparent) before
// drawing their text, matching the look of interactive widgets. `width`
// controls the backdrop width; pass a smaller value when the text is short.
void label(Context& ctx, const char* text, float width = 420.f);
void heading(Context& ctx, const char* text, float width = 420.f);
void separator(Context& ctx);

////////////////////////////////////////////////////////////////////////////////
// Interactive widgets. Return whether the user activated/changed them this
// frame. `focused` is true if the keyboard focus is on this widget — caller
// computes this from its own `selectedIdx`.

// Button: clicked or Enter while focused.
bool button(Context& ctx, const char* text, bool focused, float width = 360.f);

// Toggle: returns true if `value` flipped this frame.
bool toggle(Context& ctx, const char* text, bool& value, bool focused, float width = 360.f, bool enabled = true);

// Slider: returns true if `value` changed this frame.
//
// `editing` is in/out. While `true` the widget consumes left/right to
// step `value`. The widget itself flips it on Enter (when focused) and
// flips it back off on a second Enter. Callers that want up/down or
// escape to also exit edit mode should check `editing` themselves and
// reset it before running pane navigation. Mouse drag is always live
// and ignores the flag.
bool slider(Context&    ctx,
            const char* text,
            float&      value,
            float       min,
            float       max,
            float       step,
            bool        focused,
            bool&       editing,
            float       width = 360.f);

// One-line text input. Returns true if the buffer changed this frame.
// `submitted` is set to true on Enter while focused.
bool textField(Context& ctx, const char* labelText, sf::base::String& buf, bool focused, bool& submitted, float width = 360.f);

// Generic clickable rectangle. Returns true on click inside `rect` this frame.
struct RowResult
{
    bool clicked{};
    bool hovered{};
};
RowResult clickArea(const Context& ctx, sf::Rect2f rect);

////////////////////////////////////////////////////////////////////////////////
// Small shared utilities. All call sites that needed pill rendering, ad-hoc
// snprintf-then-label, or case-insensitive substring search can use these
// instead of duplicating the boilerplate.

// Draws an accent-colored selection pill of the configured row height at
// `topLeft`, padded `8` pixels wider than `width` to leave breathing room
// around the row text.
void pill(Context& ctx, sf::Vec2f topLeft, float width);
void pill(Context& ctx, sf::Vec2f topLeft, float width, sf::Color fill);

// printf-style `label`. The buffer is fixed-size (256 bytes); excess output
// is truncated, never reallocated.
[[gnu::format(printf, 2, 3)]]
void labelf(Context& ctx, const char* fmt, ...);

// Case-insensitive substring match, ASCII-only. Empty `needle` matches.
[[nodiscard]] bool containsCI(sf::base::StringView haystack, sf::base::StringView needle) noexcept;

// Apply up/down navigation to a wrap-around list selection. Plays a "beep"
// via `svc.playSound` whenever the index moves. Returns true on change.
// `n` is the list length; no-op if `n <= 0`.
bool navigateList(Context& ctx, Services& svc, int& idx, int n);

////////////////////////////////////////////////////////////////////////////////
// Pane helpers. A "pane" is a vertical column of focusable rows: the new
// menu has nine of them (Main items, Online actions, Options categories +
// items, LevelSelect packs/levels/actions, Workshop sidebar/list/actions).
// Every pane needs the same three pieces — animated pill, active-aware
// up/down navigation, and (for multi-pane screens) left/right focus
// movement. Inlining them at every site is what produced the
// "active-but-still-using-the-default-pill-color" bug; each helper here
// collapses the per-screen boilerplate to one line.

// Animates `pillY` toward `idx * ctx.rowHeight` and draws a row-height
// selection pill at `topLeft + (0, pillY)`. The pill uses `colAccent`
// when `active`, the desaturated accent otherwise.
void animatedPill(Context& ctx, sf::Vec2f topLeft, float width, int idx, float& pillY, bool active);

// Active-aware list navigation. Identical to `navigateList(ctx, svc, idx, n)`
// when `active` is true; a no-op when false (so multi-pane screens can
// loop over their panes and only the focused one consumes up/down).
// Returns true if `idx` changed.
bool navigatePane(Context& ctx, Services& svc, int& idx, int n, bool active);

// Lightweight handler for left/right arrow pane switching. `activePane` is
// clamped to `[0, paneCount)`; left moves it down by one, right moves it
// up by one (no wrap). Plays a "beep" through `svc` on change. Returns
// true if `activePane` changed. Screens that bind left/right to other
// actions (e.g. LevelSelect's DIFFICULTY arrows) skip this helper and
// roll their own switching logic.
bool paneSwitchLeftRight(Context& ctx, Services& svc, int& activePane, int paneCount);

// Anchors a screen at `origin`, draws the heading, and leaves the cursor
// positioned for the first widget. Replaces the cursor-reset + heading
// boilerplate every screen used to repeat. `origin` is in target pixels.
void beginScreen(Context& ctx, sf::Vec2f origin, const char* title);

// Returns the viewport size as a `Vec2f`. Trivial helper that removes the
// `static_cast<float>(ctx.target->getSize().x)` repetition seen at the
// top of every screen.
[[nodiscard]] sf::Vec2f viewportSize(const Context& ctx) noexcept;

// Canonical screen origin so headings and first-row content line up
// horizontally across the menu stack. Used by every screen via
// `beginScreen(ctx, screenOrigin(ctx), title)`.
[[nodiscard]] sf::Vec2f screenOrigin(const Context& ctx) noexcept;

// Truncates `buf` in place so it renders within `width` pixels at the
// given font size, appending "..." when something is cut. Approximate —
// uses a fixed per-glyph advance — but good enough to keep custom text
// from overflowing row backdrops. `label`/`heading` apply this internally.
void truncateToFit(char* buf, float width, float fontSize) noexcept;

// Draws a label-style row with text + backdrop tinted by a low alpha, then
// advances the cursor. Use as a "there's more below" hint at the bottom
// of a windowed list.
void drawFadedPeekRow(Context& ctx, const char* text, float width);

// Returns a desaturated/grayscale version of the given color, preserving
// alpha. Use for pills on inactive panes so the user can tell which side
// has keyboard focus.
[[nodiscard]] sf::Color desaturate(sf::Color c) noexcept;

// Routes a sound effect through `ctx.services->playSound` when available.
// No-op if the host hasn't installed a services pointer (unit tests etc.).
void playUiSound(const Context& ctx, sf::base::StringView name);

// Draws arbitrary text at `pos` (model space) with the given character
// size, in `ctx.colHighlight`, with the standard magenta outline so the
// post-process gradient pass picks it up. For one-off titles / labels
// that don't fit the row-based widget cadence. Does not advance the
// cursor — callers position themselves manually.
void text(Context& ctx, sf::Vec2f pos, const char* str, float charSize);

// Local bounds of the text `text()` would draw with the same arguments,
// without rendering it. Use to align multi-line titles or to size
// backdrops behind text. The rect's `position` carries SFML's natural
// glyph offset (typically a small negative y for ascenders); add it to
// `pos` to get the actual drawn pixel rect.
[[nodiscard]] sf::Rect2f measureText(const Context& ctx, const char* str, float charSize);

// Refresh `ctx.textCenterOffsetY` from the current font + `fontSize`.
// Call once after the host fills `ctx.font` / `ctx.fontSize` (typically
// at the top of each frame). Cheap — measures one reference glyph.
void recomputeTextMetrics(Context& ctx);

// Y-coordinate at which to draw text so that it sits vertically
// centered in `rowRect`. Uses `ctx.textCenterOffsetY`; pass `charSize`
// when drawing at something other than `ctx.fontSize` (e.g. the slider's
// numeric readout) so the offset scales proportionally.
[[nodiscard]] float rowTextY(const Context& ctx, sf::Rect2f rowRect, float charSize = 0.f) noexcept;

} // namespace hg::ui

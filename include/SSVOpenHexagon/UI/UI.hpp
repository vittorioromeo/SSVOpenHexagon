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

////////////////////////////////////////////////////////////////////////////////
// Per-frame snapshot of input the UI cares about. Filled once at the top of
// the menu update; widgets read it but never poll SFML directly.

struct Input
{
    sf::Vec2f mousePos{};

    bool mousePressed{}; //!< edge: mouse button just pressed this frame
    bool mouseDown{};    //!< level: mouse button held this frame

    bool up{};       //!< edge: navigate up
    bool down{};     //!< edge: navigate down
    bool left{};     //!< edge: decrease / move left
    bool right{};    //!< edge: increase / move right
    bool enter{};    //!< edge: activate selection
    bool escape{};   //!< edge: back / cancel
    bool backspace{};//!< edge: erase one character (text fields)

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

    // Time delta of the current frame, in seconds. Animation helpers read this.
    float dt{1.f / 60.f};

    // Layout cursor. Widgets advance it as they draw. `origin` is what
    // `newColumn()` / `newLine()` reset cursor.x to.
    sf::Vec2f cursor{};
    sf::Vec2f origin{};

    // Default row height in screen-space pixels. Widgets respect this when
    // advancing the cursor.
    float rowHeight{34.f};

    // Theme. Plain fields; mutate directly to override per-screen.
    sf::Color colText      {220, 220, 220, 255};
    sf::Color colTextDim   {130, 130, 130, 255};
    sf::Color colHighlight {255, 255, 255, 255};
    sf::Color colAccent    { 80, 170, 255, 255};
    sf::Color colRow       { 18,  18,  22, 200};
    sf::Color colRowFocused{ 80, 170, 255,  90};

    float fontSize{22.f};
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

void newLine  (Context& ctx, float dyExtra = 0.f);
void newColumn(Context& ctx, float dx);
void indent   (Context& ctx, float dx);

// Rectangle of the current row at the cursor, given a width.
sf::Rect2f rowRect(const Context& ctx, float width) noexcept;

////////////////////////////////////////////////////////////////////////////////
// Display-only widgets.

void label    (Context& ctx, const char* text);
void heading  (Context& ctx, const char* text);
void separator(Context& ctx);

////////////////////////////////////////////////////////////////////////////////
// Interactive widgets. Return whether the user activated/changed them this
// frame. `focused` is true if the keyboard focus is on this widget — caller
// computes this from its own `selectedIdx`.

// Button: clicked or Enter while focused.
bool button(Context& ctx, const char* text, bool focused, float width = 360.f);

// Toggle: returns true if `value` flipped this frame.
bool toggle(Context& ctx, const char* text, bool& value, bool focused, float width = 360.f);

// Slider: returns true if `value` changed this frame.
bool slider(Context& ctx, const char* text, float& value,
            float min, float max, float step, bool focused, float width = 360.f);

// One-line text input. Returns true if the buffer changed this frame.
// `submitted` is set to true on Enter while focused.
bool textField(Context& ctx, const char* labelText, sf::base::String& buf,
               bool focused, bool& submitted, float width = 360.f);

// Generic clickable rectangle. Returns true on click inside `rect` this frame.
struct RowResult
{
    bool clicked{};
    bool hovered{};
};
RowResult clickArea(const Context& ctx, sf::Rect2f rect);

} // namespace hg::ui

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/Math/Exp.hpp"
#include "SFML/Base/String.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace hg::ui
{

namespace
{

// Smallest movement we still consider "moving" — below this we just snap to
// avoid the asymptotic crawl of an exponential ease.
constexpr float kStepEpsilon = 0.5f;

// Pad used inside row backgrounds to keep text from touching the edges.
constexpr float kRowPad = 12.f;

void drawRect(Context& ctx, sf::Rect2f r, sf::Color fill)
{
    ctx.target->draw(sf::RectangleShapeData{
                         .position  = {r.position.x, r.position.y},
                         .fillColor = fill,
                         .size      = {r.size.x, r.size.y},
                     },
                     ctx.renderStates);
}

// Sanitize a UTF-8 input into a stack buffer of ASCII the menu font can
// definitely render. Replaces every byte ≥ 0x80 (UTF-8 continuation /
// multibyte) and every control char (< 0x20) with '?'. Workshop content,
// player-typed names, etc. can contain arbitrary Unicode that the menu
// font (Latin-only `OpenSquare`) doesn't carry glyphs for, and drawing
// such chars asserts at the SFML level. KISS: strip them at the boundary
// rather than swap fonts or use a font-fallback chain.
constexpr sf::base::SizeT kTextSanitizeBufSize = 1024;

void sanitizeForMenuFont(const char* in, char (&out)[kTextSanitizeBufSize])
{
    if (in == nullptr)
    {
        out[0] = '\0';
        return;
    }
    sf::base::SizeT i = 0;
    for (; i + 1 < kTextSanitizeBufSize && in[i] != '\0'; ++i)
    {
        const unsigned char c = static_cast<unsigned char>(in[i]);
        if (c >= 0x20 && c < 0x80)
        {
            out[i] = static_cast<char>(c);
        }
        else
        {
            out[i] = '?';
        }
    }
    out[i] = '\0';
}

void drawText(Context& ctx, sf::Vec2f pos, const char* s, sf::Color color, float sizeOverride = 0.f)
{
    char safe[kTextSanitizeBufSize];
    sanitizeForMenuFont(s, safe);
    ctx.target->draw(*ctx.font,
                     sf::TextData{
                         .position      = pos,
                         .string        = sf::UnicodeString{safe},
                         .characterSize = static_cast<unsigned int>(sizeOverride > 0.f ? sizeOverride : ctx.fontSize),
                         .fillColor     = color,
                     },
                     ctx.renderStates);
}

// True if `pt` is inside `r` (open right/bottom — matches SFML conventions).
bool pointInRect(sf::Vec2f pt, sf::Rect2f r) noexcept
{
    return pt.x >= r.position.x && pt.x < r.position.x + r.size.x &&
           pt.y >= r.position.y && pt.y < r.position.y + r.size.y;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Coordinate mapping

sf::Vec2f screenToUI(const Context& ctx, sf::Vec2f pixelPos) noexcept
{
    if (ctx.target == nullptr)
    {
        return pixelPos;
    }

    // pixel → world (undo the view's projection)
    const sf::Vec2f targetSize = ctx.target->getSize().to<sf::Vec2f>();
    const sf::Vec2f world      = ctx.renderStates.view.screenToWorld(pixelPos, targetSize);

    // world → model (undo the user-supplied transform). Identity transforms
    // are the default; this is a no-op in that common case.
    return ctx.renderStates.transform.getInverse().transformPoint(world);
}

////////////////////////////////////////////////////////////////////////////////
// Animation

bool stepToward(float& current, float target, float dt, float speed)
{
    const float diff = target - current;
    if (std::fabs(diff) <= kStepEpsilon)
    {
        if (current != target)
        {
            current = target;
            return true;
        }
        return false;
    }
    // Frame-rate-independent exponential ease: lerp by 1 - exp(-speed*dt).
    const float a = 1.f - sf::base::exp(-speed * dt);
    current += diff * a;
    return true;
}

bool stepToward(sf::Vec2f& current, sf::Vec2f target, float dt, float speed)
{
    const bool a = stepToward(current.x, target.x, dt, speed);
    const bool b = stepToward(current.y, target.y, dt, speed);
    return a || b;
}

float easeOutCubic(float t) noexcept
{
    const float u = 1.f - t;
    return 1.f - u * u * u;
}

float easeInOutCubic(float t) noexcept
{
    if (t < 0.5f)
    {
        return 4.f * t * t * t;
    }
    const float u = -2.f * t + 2.f;
    return 1.f - u * u * u * 0.5f;
}

float easeOutBack(float t) noexcept
{
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;
    const float     u  = t - 1.f;
    return 1.f + c3 * u * u * u + c1 * u * u;
}

////////////////////////////////////////////////////////////////////////////////
// Layout

void newLine(Context& ctx, float dyExtra)
{
    ctx.cursor.x = ctx.origin.x;
    ctx.cursor.y += ctx.rowHeight + dyExtra;
}

void newColumn(Context& ctx, float dx)
{
    ctx.cursor.x = ctx.origin.x + dx;
    ctx.cursor.y = ctx.origin.y;
}

void indent(Context& ctx, float dx)
{
    ctx.cursor.x += dx;
}

sf::Rect2f rowRect(const Context& ctx, float width) noexcept
{
    return sf::Rect2f{ctx.cursor, sf::Vec2f{width, ctx.rowHeight}};
}

////////////////////////////////////////////////////////////////////////////////
// Display widgets

void label(Context& ctx, const char* text)
{
    drawText(ctx,
             {ctx.cursor.x, ctx.cursor.y + (ctx.rowHeight - ctx.fontSize) * 0.5f},
             text,
             ctx.colText);
    newLine(ctx);
}

void heading(Context& ctx, const char* text)
{
    const float headingSize = ctx.fontSize * 1.6f;
    drawText(ctx, ctx.cursor, text, ctx.colHighlight, headingSize);

    // Separator under the heading.
    drawRect(ctx,
             {{ctx.cursor.x, ctx.cursor.y + headingSize + 6.f}, {420.f, 2.f}},
             ctx.colAccent);

    ctx.cursor.x = ctx.origin.x;
    ctx.cursor.y += headingSize + 16.f;
}

void separator(Context& ctx)
{
    drawRect(ctx,
             {{ctx.cursor.x, ctx.cursor.y + ctx.rowHeight * 0.5f}, {420.f, 1.f}},
             ctx.colTextDim);
    newLine(ctx, 4.f);
}

////////////////////////////////////////////////////////////////////////////////
// Interactive widgets

bool button(Context& ctx, const char* text, bool focused, float width)
{
    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused)
    {
        drawRect(ctx, r, sf::Color{255, 255, 255, 18});
    }

    drawText(ctx,
             {r.position.x + kRowPad, r.position.y + (r.size.y - ctx.fontSize) * 0.5f},
             text,
             focused ? ctx.colHighlight : ctx.colText);

    const bool activatedByKeyboard = focused && ctx.input.enter;
    const bool activatedByMouse    = hovered && ctx.input.mousePressed;

    newLine(ctx);
    return activatedByKeyboard || activatedByMouse;
}

bool toggle(Context& ctx, const char* text, bool& value, bool focused, float width)
{
    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused)
    {
        drawRect(ctx, r, sf::Color{255, 255, 255, 18});
    }

    drawText(ctx,
             {r.position.x + kRowPad, r.position.y + (r.size.y - ctx.fontSize) * 0.5f},
             text,
             focused ? ctx.colHighlight : ctx.colText);

    const char* marker      = value ? "[ ON ]" : "[ OFF ]";
    const float markerWidth = 80.f;
    drawText(ctx,
             {r.position.x + r.size.x - kRowPad - markerWidth,
              r.position.y + (r.size.y - ctx.fontSize) * 0.5f},
             marker,
             value ? ctx.colAccent : ctx.colTextDim);

    const bool activated = (focused && (ctx.input.enter || ctx.input.left || ctx.input.right)) ||
                           (hovered && ctx.input.mousePressed);

    newLine(ctx);
    if (activated)
    {
        value = !value;
        return true;
    }
    return false;
}

bool slider(Context& ctx, const char* text, float& value, float min, float max, float step, bool focused, float width)
{
    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused)
    {
        drawRect(ctx, r, sf::Color{255, 255, 255, 18});
    }

    drawText(ctx,
             {r.position.x + kRowPad, r.position.y + (r.size.y - ctx.fontSize) * 0.5f},
             text,
             focused ? ctx.colHighlight : ctx.colText);

    // Fill bar on the right half of the row.
    const float trackX     = r.position.x + r.size.x * 0.5f;
    const float trackY     = r.position.y + r.size.y * 0.5f - 2.f;
    const float trackW     = r.size.x * 0.5f - kRowPad;
    const float fillPctRaw = (max - min) > 0.f ? (value - min) / (max - min) : 0.f;
    const float fillPct    = fillPctRaw < 0.f ? 0.f : (fillPctRaw > 1.f ? 1.f : fillPctRaw);

    drawRect(ctx, {{trackX, trackY}, {trackW, 4.f}}, sf::Color{60, 60, 70, 255});
    drawRect(ctx, {{trackX, trackY}, {trackW * fillPct, 4.f}}, ctx.colAccent);

    // Numeric readout above the bar. Max ~10 chars including NUL.
    char numBuf[16] = {};
    std::snprintf(numBuf, sizeof(numBuf), "%.2f", static_cast<double>(value));
    drawText(ctx,
             {trackX, trackY - ctx.fontSize - 2.f},
             numBuf,
             focused ? ctx.colHighlight : ctx.colText,
             ctx.fontSize * 0.85f);

    bool changed = false;
    if (focused)
    {
        if (ctx.input.left)
        {
            value -= step;
            changed = true;
        }
        if (ctx.input.right)
        {
            value += step;
            changed = true;
        }
        if (changed)
        {
            if (value < min) value = min;
            if (value > max) value = max;
        }
    }

    newLine(ctx);
    return changed;
}

bool textField(Context& ctx, const char* labelText, sf::base::String& buf, bool focused, bool& submitted, float width)
{
    submitted = false;

    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused)
    {
        drawRect(ctx, r, sf::Color{255, 255, 255, 18});
    }

    drawText(ctx,
             {r.position.x + kRowPad, r.position.y + (r.size.y - ctx.fontSize) * 0.5f},
             labelText,
             focused ? ctx.colHighlight : ctx.colText);

    // Draw the buffer contents to the right of the label (rough horizontal
    // offset; users that want exact alignment can custom-render).
    drawText(ctx,
             {r.position.x + kRowPad + 180.f, r.position.y + (r.size.y - ctx.fontSize) * 0.5f},
             buf.data(),
             ctx.colText);

    bool changed = false;
    if (focused)
    {
        if (ctx.input.backspace && buf.size() > 0)
        {
            buf.erase(buf.size() - 1, 1);
            changed = true;
        }
        if (ctx.input.typedChars[0] != '\0')
        {
            buf += ctx.input.typedChars;
            changed = true;
        }
        if (ctx.input.enter)
        {
            submitted = true;
        }
    }

    newLine(ctx);
    return changed;
}

RowResult clickArea(const Context& ctx, sf::Rect2f rect)
{
    const bool hovered = pointInRect(ctx.input.mousePos, rect);
    return {hovered && ctx.input.mousePressed, hovered};
}

} // namespace hg::ui

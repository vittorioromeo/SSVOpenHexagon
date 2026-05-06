// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/UI.hpp"

#include "SSVOpenHexagon/UI/Services.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/Math/Exp.hpp"
#include "SFML/Base/String.hpp"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace hg::ui
{

namespace
{

// Smallest movement we still consider "moving" — below this we just snap to
// avoid the asymptotic crawl of an exponential ease. Bumped from sub-pixel
// to a couple of pixels so the tail of the animation never feels like a
// crawl; sub-pixel residue is imperceptible anyway.
constexpr float kStepEpsilon = 2.f;

// Minimum step magnitude (in units per second) applied on top of the
// exponential ease. Without it, exponential decay's per-frame step shrinks
// toward zero as we approach the target, producing the "slows to a crawl"
// feeling. Acts as a velocity floor so we always close the last few pixels
// at a perceptible rate.
constexpr float kMinStepPerSecond = 240.f;

// Pad used inside row backgrounds to keep text from touching the edges.
constexpr float kRowPad = 12.f;

// Fraction of the font size used as text outline thickness. The accent
// outline is the gradient sentinel — kept thin so glyph interiors stay
// readable, beefy enough that the gradient pass has room to paint. Both
// `drawText` and `measureText` use this so measured bounds match what's
// actually drawn (outline pushes the bounding rect outward).
constexpr float kOutlineFactor = 0.025f;

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
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (c >= 0x20 && c < 0x80)
        {
            // Force-uppercase: the menu's design language is all-caps,
            // so user-supplied strings (level names, pack names, typed
            // input, version "v2.2.0", etc.) are normalized here once
            // instead of being uppercased at every call site.
            if (c >= 'a' && c <= 'z')
            {
                c = static_cast<unsigned char>(c - 'a' + 'A');
            }
            out[i] = static_cast<char>(c);
        }
        else
        {
            out[i] = '?';
        }
    }
    out[i] = '\0';
}

void drawText(Context& ctx, sf::Vec2f pos, const char* s, sf::Color color,
              float sizeOverride = 0.f, float maxWidth = 0.f)
{
    char safe[kTextSanitizeBufSize];
    sanitizeForMenuFont(s, safe);
    const float effSize = sizeOverride > 0.f ? sizeOverride : ctx.fontSize;
    if (maxWidth > 0.f)
    {
        truncateToFit(safe, maxWidth, effSize);
    }
    // Outline thickness scales with font size so headings get a beefier
    // border than body text. The accent color is the gradient sentinel —
    // the post-process shader replaces it with an animated noise gradient.
    const float outline = effSize * kOutlineFactor;
    ctx.target->draw(*ctx.font,
                     sf::TextData{
                         .position         = pos,
                         .string           = sf::UnicodeString{safe},
                         .characterSize    = static_cast<unsigned int>(effSize),
                         .fillColor        = color,
                         .outlineColor     = ctx.colAccent,
                         .outlineThickness = outline,
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

    // pixel → world (undo the view's projection). Mirror what
    // `RenderTarget::draw` does: a default-constructed `view` is replaced
    // with one spanning the target's size, so we must do the same here or
    // `screenToWorld` operates on a zero-sized view and returns garbage.
    const sf::Vec2f targetSize = ctx.target->getSize().to<sf::Vec2f>();
    const sf::View  effectiveView = (ctx.renderStates.view == sf::View{})
                                        ? sf::View::fromScreenSize(targetSize)
                                        : ctx.renderStates.view;
    const sf::Vec2f world         = effectiveView.screenToWorld(pixelPos, targetSize);

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
    const float a       = 1.f - sf::base::exp(-speed * dt);
    const float expStep = diff * a;

    // Velocity floor — guarantee we move at least `kMinStepPerSecond * dt`
    // toward the target. Hides the asymptotic slowdown of exponential
    // decay, so the cursor doesn't crawl through the last few pixels.
    const float floor   = kMinStepPerSecond * dt;
    float       step    = expStep;
    if (std::fabs(step) < floor)
    {
        step = (diff > 0.f) ? floor : -floor;
    }

    // Don't overshoot the target.
    if ((diff > 0.f && step > diff) || (diff < 0.f && step < diff))
    {
        step = diff;
    }

    current += step;
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

void label(Context& ctx, const char* text, float width)
{
    const sf::Rect2f r = rowRect(ctx, width);
    drawRect(ctx, r, ctx.colRow);

    char safe[kTextSanitizeBufSize];
    sanitizeForMenuFont(text, safe);
    truncateToFit(safe, r.size.x - kRowPad * 2.f, ctx.fontSize);

    drawText(ctx,
             {r.position.x + kRowPad, rowTextY(ctx, r)},
             safe,
             ctx.colText);
    newLine(ctx);
}

void heading(Context& ctx, const char* text, float width)
{
    const float      headingSize = ctx.fontSize * 1.6f;
    const float      bgHeight    = headingSize + 12.f;
    const sf::Rect2f bg{ctx.cursor, {width, bgHeight}};

    drawRect(ctx, bg, ctx.colRow);

    char safe[kTextSanitizeBufSize];
    sanitizeForMenuFont(text, safe);
    truncateToFit(safe, width - kRowPad * 2.f, headingSize);

    drawText(ctx, {bg.position.x + kRowPad, rowTextY(ctx, bg, headingSize)},
             safe, ctx.colHighlight, headingSize);

    // Accent stripe under the heading.
    drawRect(ctx,
             {{ctx.cursor.x, ctx.cursor.y + bgHeight}, {width, 2.f}},
             ctx.colAccent);

    ctx.cursor.x = ctx.origin.x;
    ctx.cursor.y += bgHeight + 14.f;
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

// Theme-aware hover tint. Picks black-on-light or white-on-dark depending
// on the row backdrop's luminance, with enough alpha to actually read as
// feedback on both ends of the theme range.
void drawHoverOverlay(Context& ctx, sf::Rect2f r)
{
    const sf::Color& bg = ctx.colRow;
    const int        bgLuma = (static_cast<int>(bg.r) * 299 +
                               static_cast<int>(bg.g) * 587 +
                               static_cast<int>(bg.b) * 114) /
                              1000;
    drawRect(ctx, r,
             (bgLuma > 128)
                 ? sf::Color{  0,   0,   0,  60}    // light bg → dark hover tint
                 : sf::Color{255, 255, 255,  60});  // dark bg  → light hover tint
}

bool button(Context& ctx, const char* text, bool focused, float width)
{
    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused) drawHoverOverlay(ctx, r);

    drawText(ctx,
             {r.position.x + kRowPad, rowTextY(ctx, r)},
             text,
             focused ? ctx.colHighlight : ctx.colText,
             0.f,
             width - kRowPad * 2.f);

    const bool activated = (focused && ctx.input.enter) ||
                           (hovered && ctx.input.mousePressed);

    newLine(ctx);
    if (activated) playUiSound(ctx, "select.ogg");
    return activated;
}

bool toggle(Context& ctx, const char* text, bool& value, bool focused, float width)
{
    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused) drawHoverOverlay(ctx, r);

    constexpr float kMarkerW = 80.f;
    drawText(ctx,
             {r.position.x + kRowPad, rowTextY(ctx, r)},
             text,
             focused ? ctx.colHighlight : ctx.colText,
             0.f,
             r.size.x - kRowPad * 2.f - kMarkerW - 8.f);

    const char* marker = value ? "[ ON ]" : "[ OFF ]";
    drawText(ctx,
             {r.position.x + r.size.x - kRowPad - kMarkerW,
              rowTextY(ctx, r)},
             marker,
             value ? ctx.colAccent : ctx.colTextDim);

    const bool activated = (focused && (ctx.input.enter || ctx.input.left || ctx.input.right)) ||
                           (hovered && ctx.input.mousePressed);

    newLine(ctx);
    if (activated)
    {
        value = !value;
        playUiSound(ctx, "select.ogg");
        return true;
    }
    return false;
}

bool slider(Context& ctx, const char* text, float& value, float min, float max, float step, bool focused, float width)
{
    const sf::Rect2f r       = rowRect(ctx, width);
    const bool       hovered = pointInRect(ctx.input.mousePos, r);

    drawRect(ctx, r, focused ? ctx.colRowFocused : ctx.colRow);
    if (hovered && !focused) drawHoverOverlay(ctx, r);

    drawText(ctx,
             {r.position.x + kRowPad, rowTextY(ctx, r)},
             text,
             focused ? ctx.colHighlight : ctx.colText,
             0.f,
             r.size.x * 0.5f - kRowPad * 2.f);

    // Right half of the row: numeric readout (left), then track (right).
    // Both are vertically centered so neither leaks above or below the row.
    constexpr float kNumericW   = 60.f;
    const float     numericX    = r.position.x + r.size.x * 0.5f;
    const float     trackX      = numericX + kNumericW;
    const float     trackY      = r.position.y + r.size.y * 0.5f - 2.f;
    const float     trackW      = r.size.x * 0.5f - kNumericW - kRowPad;
    const float     fillPctRaw  = (max - min) > 0.f ? (value - min) / (max - min) : 0.f;
    const float     fillPct     = fillPctRaw < 0.f ? 0.f : (fillPctRaw > 1.f ? 1.f : fillPctRaw);
    const float     numFontSize = ctx.fontSize * 0.85f;

    char numBuf[16];
    std::snprintf(numBuf, sizeof(numBuf), "%.2f", static_cast<double>(value));
    drawText(ctx,
             {numericX, rowTextY(ctx, r, numFontSize)},
             numBuf,
             focused ? ctx.colHighlight : ctx.colText,
             numFontSize);

    drawRect(ctx, {{trackX, trackY}, {trackW, 4.f}},          sf::Color{60, 60, 70, 255});
    drawRect(ctx, {{trackX, trackY}, {trackW * fillPct, 4.f}}, ctx.colAccent);

    bool changed = false;
    if (focused)
    {
        if (ctx.input.left)  { value -= step; changed = true; }
        if (ctx.input.right) { value += step; changed = true; }
    }

    // Mouse drag on the track: while the LMB is held over the row, set the
    // value from the cursor's x-position within the track. Snap to `step`
    // and clamp to `[min, max]`. Stateless — relies only on `mouseDown` +
    // `hovered`, which is fine for a list of sliders since the user can't
    // be over two at once.
    if (hovered && ctx.input.mouseDown && trackW > 0.f && (max - min) > 0.f)
    {
        const float pct      = (ctx.input.mousePos.x - trackX) / trackW;
        const float clamped  = pct < 0.f ? 0.f : (pct > 1.f ? 1.f : pct);
        const float rawValue = min + clamped * (max - min);

        // Quantize to the requested step.
        const float quant = (step > 0.f)
                                ? min + std::round((rawValue - min) / step) * step
                                : rawValue;
        if (quant != value)
        {
            value   = quant;
            changed = true;
        }
    }

    if (changed)
    {
        if (value < min) value = min;
        if (value > max) value = max;
        playUiSound(ctx, "beep.ogg"); // each tick beeps; quieter than `select.ogg`
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
    if (hovered && !focused) drawHoverOverlay(ctx, r);

    constexpr float kInputOffsetX = 180.f;
    drawText(ctx,
             {r.position.x + kRowPad, rowTextY(ctx, r)},
             labelText,
             focused ? ctx.colHighlight : ctx.colText,
             0.f,
             kInputOffsetX - kRowPad);

    // Draw the buffer contents to the right of the label (rough horizontal
    // offset; users that want exact alignment can custom-render).
    drawText(ctx,
             {r.position.x + kRowPad + kInputOffsetX, rowTextY(ctx, r)},
             buf.data(),
             ctx.colText,
             0.f,
             r.size.x - kRowPad * 2.f - kInputOffsetX);

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
            playUiSound(ctx, "select.ogg");
        }
    }

    newLine(ctx);
    if (changed) playUiSound(ctx, "beep.ogg"); // typing / erasing
    return changed;
}

RowResult clickArea(const Context& ctx, sf::Rect2f rect)
{
    const bool hovered = pointInRect(ctx.input.mousePos, rect);
    return {hovered && ctx.input.mousePressed, hovered};
}

////////////////////////////////////////////////////////////////////////////////
// Shared utilities

void pill(Context& ctx, sf::Vec2f topLeft, float width, sf::Color fill)
{
    drawRect(ctx, {{topLeft.x - 8.f, topLeft.y}, {width + 16.f, ctx.rowHeight}}, fill);
}

void pill(Context& ctx, sf::Vec2f topLeft, float width)
{
    pill(ctx, topLeft, width, ctx.colAccent);
}

void labelf(Context& ctx, const char* fmt, ...)
{
    char    buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    label(ctx, buf);
}

bool navigateList(Context& ctx, Services& svc, int& idx, int n)
{
    if (n <= 0) return false;
    bool changed = false;
    if (ctx.input.up)   { idx = (idx + n - 1) % n; changed = true; }
    if (ctx.input.down) { idx = (idx + 1) % n;     changed = true; }
    if (changed && svc.playSound) svc.playSound("beep.ogg");
    return changed;
}

void beginScreen(Context& ctx, sf::Vec2f origin, const char* title)
{
    ctx.cursor = ctx.origin = origin;
    heading(ctx, title);
}

sf::Vec2f viewportSize(const Context& ctx) noexcept
{
    // Prefer the size of the view the host installed on `renderStates`.
    // Screens use this for layout (anchoring, percentages, mouse hit-test
    // bounds), so they need to read the *virtual* drawing space — which
    // matches the host's design coordinate system — rather than raw window
    // pixels. The default (uninitialized) `View{}` falls back to the
    // target size, mirroring what `RenderTarget::draw` does at draw time.
    const sf::View& view = ctx.renderStates.view;
    if (view != sf::View{}) return view.size;

    if (ctx.target == nullptr) return {0.f, 0.f};
    return ctx.target->getSize().to<sf::Vec2f>();
}

sf::Vec2f screenOrigin(const Context& /*ctx*/) noexcept
{
    // The legacy logo sprite has been retired — the Main screen now draws
    // its own outlined "OPEN HEXAGON" title at this anchor, and every
    // sub-screen aligns its heading here too. Virtual pixel space, holds
    // at any window resolution thanks to the overlay view.
    return {20.f, 20.f};
}

void drawFadedPeekRow(Context& ctx, const char* text, float width)
{
    // Save & dim text/bg colors so we don't permanently mutate the theme.
    // Alpha at ~1/3 of the original is enough to read as "ghost row".
    const sf::Color textBefore = ctx.colText;
    const sf::Color rowBefore  = ctx.colRow;
    const auto      fadeAlpha  = [](sf::base::U8 a) {
        return static_cast<sf::base::U8>(static_cast<unsigned int>(a) / 3u);
    };
    ctx.colText = sf::Color{textBefore.r, textBefore.g, textBefore.b, fadeAlpha(textBefore.a)};
    ctx.colRow  = sf::Color{rowBefore.r,  rowBefore.g,  rowBefore.b,  fadeAlpha(rowBefore.a)};

    label(ctx, text, width);

    ctx.colText = textBefore;
    ctx.colRow  = rowBefore;
}

void playUiSound(const Context& ctx, sf::base::StringView name)
{
    if (ctx.services && ctx.services->playSound) ctx.services->playSound(name);
}

void text(Context& ctx, sf::Vec2f pos, const char* str, float charSize)
{
    drawText(ctx, pos, str, ctx.colHighlight, charSize);
}

sf::Rect2f measureText(const Context& ctx, const char* str, float charSize)
{
    if (ctx.font == nullptr || str == nullptr || charSize <= 0.f)
    {
        return {};
    }

    char safe[kTextSanitizeBufSize];
    sanitizeForMenuFont(str, safe);

    return sf::TextUtils::precomputeTextLocalBounds(
        *ctx.font,
        sf::TextData{
            .string           = sf::UnicodeString{safe},
            .characterSize    = static_cast<unsigned int>(charSize),
            .outlineThickness = charSize * kOutlineFactor,
        });
}

void recomputeTextMetrics(Context& ctx)
{
    if (ctx.font == nullptr || ctx.fontSize <= 0.f)
    {
        ctx.textCenterOffsetY = 0.f;
        return;
    }

    // Reference glyph for the cap-line. Most menu labels are uppercase,
    // so centering on a cap-only reference matches the dominant visual
    // line; the occasional descender (e.g. lowercase 'g' / 'y') dips
    // slightly below center, which is the standard typography behavior.
    const sf::Rect2f bounds = measureText(ctx, "M", ctx.fontSize);
    ctx.textCenterOffsetY   = bounds.position.y + bounds.size.y * 0.5f;
}

float rowTextY(const Context& ctx, sf::Rect2f rowRect, float charSize) noexcept
{
    const float scale = (charSize <= 0.f || ctx.fontSize <= 0.f)
                            ? 1.f
                            : (charSize / ctx.fontSize);
    return rowRect.position.y + rowRect.size.y * 0.5f - ctx.textCenterOffsetY * scale;
}

sf::Color desaturate(sf::Color c) noexcept
{
    const sf::base::U8 g = static_cast<sf::base::U8>(
        (static_cast<int>(c.r) * 299 +
         static_cast<int>(c.g) * 587 +
         static_cast<int>(c.b) * 114) / 1000);
    return sf::Color{g, g, g, c.a};
}

void truncateToFit(char* buf, float width, float fontSize) noexcept
{
    if (buf == nullptr || width <= 0.f) return;

    // The OpenSquare menu font is roughly 0.6em wide per glyph (monospace).
    const float charW = fontSize * 0.6f;
    if (charW <= 0.f) return;

    const sf::base::SizeT len     = std::strlen(buf);
    const sf::base::SizeT maxFull = static_cast<sf::base::SizeT>(width / charW);
    if (len <= maxFull) return; // already fits

    constexpr sf::base::SizeT kEllipsisLen = 3;
    if (maxFull <= kEllipsisLen)
    {
        buf[maxFull] = '\0'; // pathologically narrow — hard-clip
        return;
    }
    const sf::base::SizeT keep = maxFull - kEllipsisLen;
    buf[keep + 0] = '.';
    buf[keep + 1] = '.';
    buf[keep + 2] = '.';
    buf[keep + 3] = '\0';
}

bool containsCI(sf::base::StringView haystack, sf::base::StringView needle) noexcept
{
    if (needle.empty()) return true;
    if (haystack.size() < needle.size()) return false;

    const auto toLower = [](char c) -> char {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    };

    const auto hSize = haystack.size();
    const auto nSize = needle.size();
    for (sf::base::SizeT i = 0; i + nSize <= hSize; ++i)
    {
        bool matched = true;
        for (sf::base::SizeT j = 0; j < nSize; ++j)
        {
            if (toLower(haystack.data()[i + j]) != toLower(needle.data()[j]))
            {
                matched = false;
                break;
            }
        }
        if (matched) return true;
    }
    return false;
}

} // namespace hg::ui

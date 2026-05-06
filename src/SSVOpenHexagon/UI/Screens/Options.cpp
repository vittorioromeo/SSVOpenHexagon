// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

namespace hg::ui
{

namespace
{

// Two flavors of option:
//   ToggleItem  — read+write a bool
//   SliderItem  — read+write a float (with min/max/step)
// We keep two parallel arrays per category instead of a tagged union to keep
// each loop's code dead-simple. Adding a new item = adding one line.
struct ToggleItem
{
    const char* label;
    bool (*get)();
    void (*set)(bool);
};

struct SliderItem
{
    const char* label;
    float (*get)();
    void (*set)(float);
    float min;
    float max;
    float step;
};

////////////////////////////////////////////////////////////////////////////////
// GAMEPLAY

constexpr ToggleItem kGameplayToggles[] = {
    {"NO PULSE", &Config::getNoPulse, &Config::setNoPulse},
    {"NO ROTATION", &Config::getNoRotation, &Config::setNoRotation},
    {"NO BACKGROUND", &Config::getNoBackground, &Config::setNoBackground},
    {"BLACK & WHITE", &Config::getBlackAndWhite, &Config::setBlackAndWhite},
    {"AUTO RESTART", &Config::getAutoRestart, &Config::setAutoRestart},
    {"INVINCIBLE", &Config::getInvincible, &Config::setInvincible},
    {"ROTATE TO START", &Config::getRotateToStart, &Config::setRotateToStart},
};

constexpr int kGameplayToggleCount = static_cast<int>(sizeof(kGameplayToggles) / sizeof(*kGameplayToggles));

////////////////////////////////////////////////////////////////////////////////
// AUDIO

constexpr ToggleItem kAudioToggles[] = {
    {"NO SOUND", &Config::getNoSound, &Config::setNoSound},
    {"NO MUSIC", &Config::getNoMusic, &Config::setNoMusic},
    {"MUSIC SPEED SYNC", &Config::getMusicSpeedDMSync, &Config::setMusicSpeedDMSync},
};
constexpr int kAudioToggleCount = static_cast<int>(sizeof(kAudioToggles) / sizeof(*kAudioToggles));

constexpr SliderItem kAudioSliders[] = {
    {"SOUND VOLUME", &Config::getSoundVolume, &Config::setSoundVolume, 0.f, 100.f, 5.f},
    {"MUSIC VOLUME", &Config::getMusicVolume, &Config::setMusicVolume, 0.f, 100.f, 5.f},
    {"MUSIC SPEED", &Config::getMusicSpeedMult, &Config::setMusicSpeedMult, 0.5f, 2.f, 0.05f},
};
constexpr int kAudioSliderCount = static_cast<int>(sizeof(kAudioSliders) / sizeof(*kAudioSliders));

////////////////////////////////////////////////////////////////////////////////
// GRAPHICS

constexpr ToggleItem kGraphicsToggles[] = {
    {"FLASH", &Config::getFlash, &Config::setFlash},
    {"3D EFFECT", &Config::get3D, &Config::set3D},
    {"SHADERS", &Config::getShaders, &Config::setShaders},
    {"SHOW FPS", &Config::getShowFPS, &Config::setShowFPS},
    {"SHOW TIMER", &Config::getShowTimer, &Config::setShowTimer},
    {"SHOW STATUS TEXT", &Config::getShowStatusText, &Config::setShowStatusText},
    {"SHOW LEVEL INFO", &Config::getShowLevelInfo, &Config::setShowLevelInfo},
    {"SHOW KEY ICONS", &Config::getShowKeyIcons, &Config::setShowKeyIcons},
    {"DARKEN UNEVEN BG CHUNK", &Config::getDarkenUnevenBackgroundChunk, &Config::setDarkenUnevenBackgroundChunk},
    {"DRAW TEXT OUTLINES", &Config::getDrawTextOutlines, &Config::setDrawTextOutlines},
    {"PLAYER TRAIL", &Config::getShowPlayerTrail, &Config::setShowPlayerTrail},
    {"SWAP PARTICLES", &Config::getShowSwapParticles, &Config::setShowSwapParticles},
};
constexpr int kGraphicsToggleCount = static_cast<int>(sizeof(kGraphicsToggles) / sizeof(*kGraphicsToggles));

constexpr SliderItem kGraphicsSliders[] = {
    {"CAMERA SHAKE", &Config::getCameraShakeMultiplier, &Config::setCameraShakeMultiplier, 0.f, 3.f, 0.1f},
    {"ANGLE TILT INTENSITY", &Config::getAngleTiltIntensity, &Config::setAngleTiltIntensity, 0.f, 3.f, 0.1f},
    {"TEXT SCALING", &Config::getTextScaling, &Config::setTextScaling, 0.5f, 2.f, 0.05f},
};
constexpr int kGraphicsSliderCount = static_cast<int>(sizeof(kGraphicsSliders) / sizeof(*kGraphicsSliders));

////////////////////////////////////////////////////////////////////////////////
// ADVANCED

constexpr ToggleItem kAdvancedToggles[] = {
    {"USE LUA FILE CACHE", &Config::getUseLuaFileCache, &Config::setUseLuaFileCache},
    {"DISABLE GAME RENDERING", &Config::getDisableGameRendering, &Config::setDisableGameRendering},
    {"DEBUG MODE", &Config::getDebug, &Config::setDebug},
};
constexpr int kAdvancedToggleCount = static_cast<int>(sizeof(kAdvancedToggles) / sizeof(*kAdvancedToggles));

////////////////////////////////////////////////////////////////////////////////
// Categories

struct CategoryDef
{
    const char*       name;
    const ToggleItem* toggles;
    int               toggleCount;
    const SliderItem* sliders;
    int               sliderCount;
};

constexpr CategoryDef kCategories[] = {
    {"GAMEPLAY", kGameplayToggles, kGameplayToggleCount, nullptr, 0},
    {"AUDIO", kAudioToggles, kAudioToggleCount, kAudioSliders, kAudioSliderCount},
    {"GRAPHICS", kGraphicsToggles, kGraphicsToggleCount, kGraphicsSliders, kGraphicsSliderCount},
    {"ADVANCED", kAdvancedToggles, kAdvancedToggleCount, nullptr, 0},
};
constexpr int kCategoryCount = static_cast<int>(sizeof(kCategories) / sizeof(*kCategories));

[[nodiscard]] int totalItems(const CategoryDef& c) noexcept
{
    return c.toggleCount + c.sliderCount;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////

void drawOptionsScreen(Context& ctx, App& app, Services& svc)
{
    OptionsScreenState& s = app.options;

    // The category list has a synthetic "BACK" row appended at the end —
    // keyboard-navigable like any other category, but activating it pops
    // the screen instead of populating the items pane. `kBackRow` is the
    // category-list index of that row; `kCatNavCount` is the total number
    // of navigable rows including BACK.
    constexpr int kBackRow     = kCategoryCount;
    constexpr int kCatNavCount = kCategoryCount + 1;

    // Clamp category index in case its memory is stale.
    if (s.selectedCategory < 0 || s.selectedCategory >= kCatNavCount)
    {
        s.selectedCategory = 0;
    }

    // BACK row has no items; for any real category, take the items as-is.
    const bool         onBackRow = (s.selectedCategory == kBackRow);
    const CategoryDef& cat       = onBackRow ? kCategories[0] : kCategories[s.selectedCategory];
    const int          itemN     = onBackRow ? 0 : totalItems(cat);

    // ---- Input ------------------------------------------------------------
    // Escape pops the item-pane focus first, then the screen.
    if (ctx.input.escape)
    {
        if (s.selectedItem >= 0) { s.selectedItem = -1; }
        else                     { goBack(app); return; }
    }

    const int prevCategory = s.selectedCategory;

    if (s.selectedItem < 0)
    {
        // Focus on category list (incl. the synthetic BACK row).
        navigateList(ctx, svc, s.selectedCategory, kCatNavCount);
        if (s.selectedCategory == kBackRow)
        {
            if (ctx.input.right || ctx.input.enter)
            {
                goBack(app);
                return;
            }
        }
        else if (ctx.input.right || ctx.input.enter)
        {
            // Move focus into the items pane.
            s.selectedItem = 0;
        }
    }
    else if (itemN <= 0)
    {
        s.selectedItem = -1;
    }
    else
    {
        navigateList(ctx, svc, s.selectedItem, itemN);
        if (ctx.input.left) s.selectedItem = -1; // back to category list
    }

    stepToward(s.categorySelectionY,
               static_cast<float>(s.selectedCategory) * ctx.rowHeight,
               ctx.dt, 256.f);

    // Snap the item pill (no animation) when the category changes — the
    // visible items list has changed entirely, so a smooth transition would
    // travel through unrelated rows. Also re-clamp `selectedItem` because
    // the new category may have fewer items.
    if (prevCategory != s.selectedCategory)
    {
        if (s.selectedItem >= itemN) s.selectedItem = itemN > 0 ? 0 : -1;
        s.itemSelectionY = (s.selectedItem >= 0) ? static_cast<float>(s.selectedItem) * ctx.rowHeight : 0.f;
    }
    else if (s.selectedItem >= 0)
    {
        stepToward(s.itemSelectionY,
                   static_cast<float>(s.selectedItem) * ctx.rowHeight,
                   ctx.dt, 256.f);
    }

    // ---- Layout -----------------------------------------------------------
    beginScreen(ctx, screenOrigin(ctx), "OPTIONS");

    const sf::Vec2f leftColTop  = {ctx.origin.x,           ctx.cursor.y};
    const sf::Vec2f rightColTop = {ctx.origin.x + 240.f,   ctx.cursor.y};

    // ---- Category list (left column) --------------------------------------
    ctx.cursor = ctx.origin = leftColTop;
    {
        // Category selection pill — dimmed when focus has moved into the
        // item pane, so it's clear which side is active.
        const sf::Color fill = (s.selectedItem < 0)
            ? ctx.colAccent
            : sf::Color{ctx.colAccent.r, ctx.colAccent.g, ctx.colAccent.b, 80};
        pill(ctx, {ctx.cursor.x, ctx.cursor.y + s.categorySelectionY}, 220.f, fill);
    }

    for (int i = 0; i < kCategoryCount; ++i)
    {
        const bool focused = (s.selectedItem < 0) && (i == s.selectedCategory);
        if (button(ctx, kCategories[i].name, focused, 220.f))
        {
            s.selectedCategory = i;
            s.selectedItem     = 0;
        }
    }

    // BACK row, drawn flush with the category list so the selection pill
    // animates over it as if it were one more category. Activating it
    // pops the screen.
    {
        const bool focused = (s.selectedItem < 0) && (s.selectedCategory == kBackRow);
        if (button(ctx, "BACK", focused, 220.f))
        {
            goBack(app);
            return;
        }
    }

    // ---- Items (right column) ---------------------------------------------
    // Suppress the entire items pane when BACK is focused — otherwise
    // it would bleed the GAMEPLAY items through (since `cat` falls back
    // to category 0 when `onBackRow` is true so `s.selectedCategory`
    // index access stays valid).
    if (onBackRow)
    {
        return;
    }

    ctx.cursor = ctx.origin = rightColTop;

    // Item selection pill, only when item pane has focus.
    if (s.selectedItem >= 0 && itemN > 0)
    {
        pill(ctx, {ctx.cursor.x, ctx.cursor.y + s.itemSelectionY}, 420.f);
    }

    int rowIdx = 0;

    for (int i = 0; i < cat.toggleCount; ++i)
    {
        const bool focused = (s.selectedItem == rowIdx);
        bool       v       = cat.toggles[i].get();
        if (toggle(ctx, cat.toggles[i].label, v, focused, 420.f))
        {
            cat.toggles[i].set(v);
        }
        ++rowIdx;
    }

    for (int i = 0; i < cat.sliderCount; ++i)
    {
        const bool focused = (s.selectedItem == rowIdx);
        float      v       = cat.sliders[i].get();
        if (slider(ctx, cat.sliders[i].label, v, cat.sliders[i].min, cat.sliders[i].max, cat.sliders[i].step, focused, 420.f))
        {
            cat.sliders[i].set(v);
        }
        ++rowIdx;
    }

    if (cat.toggleCount == 0 && cat.sliderCount == 0)
    {
        label(ctx, "(no items in this category)");
    }
}

} // namespace hg::ui

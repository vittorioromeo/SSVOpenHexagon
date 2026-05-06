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
    bool (*enabled)() = [] { return true; };
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

constexpr auto unofficialOnly = [] { return !Config::getOfficial(); };

////////////////////////////////////////////////////////////////////////////////
// GAMEPLAY

constexpr ToggleItem kGameplayToggles[] = {
    {"AUTO RESTART", &Config::getAutoRestart, &Config::setAutoRestart},
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
    // {"DISABLE GAME RENDERING", &Config::getDisableGameRendering, &Config::setDisableGameRendering},
    {"OFFICIAL MODE", &Config::getOfficial, &Config::setOfficial},
    {"DEBUG MODE", &Config::getDebug, &Config::setDebug, unofficialOnly},
    {"NO PULSE", &Config::getNoPulse, &Config::setNoPulse, unofficialOnly},
    {"NO ROTATION", &Config::getNoRotation, &Config::setNoRotation, unofficialOnly},
    {"NO BACKGROUND", &Config::getNoBackground, &Config::setNoBackground, unofficialOnly},
    {"BLACK & WHITE", &Config::getBlackAndWhite, &Config::setBlackAndWhite, unofficialOnly},
    {"INVINCIBLE", &Config::getInvincible, &Config::setInvincible, unofficialOnly},
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

    // ---- Input ------------------------------------------------------------
    // Slider edit mode hijacks navigation: while a slider is being
    // edited, left/right step the value (consumed by the widget below)
    // and up/down or escape exit edit mode WITHOUT also moving focus.
    // We snapshot it at frame start because the widget may flip it
    // mid-frame (Enter toggles), and we don't want that flip to retro-
    // actively gate this frame's navigation.
    const bool wasEditingSlider = s.sliderEditing;
    if (wasEditingSlider && (ctx.input.escape || ctx.input.up || ctx.input.down))
    {
        s.sliderEditing = false;
    }

    // Escape pops the item-pane focus first, then the screen. Skipped
    // when we just used escape to exit slider edit mode.
    if (ctx.input.escape && !wasEditingSlider)
    {
        if (s.selectedItem >= 0)
        {
            s.selectedItem = -1;
        }
        else
        {
            goBack(app);
            return;
        }
    }

    // Two-pane focus: 0 = categories, 1 = items. Encoded as a derived
    // value off `selectedItem` (the existing sentinel) so screen state
    // stays in one field; the helpers consume an int the same way.
    const int  prevCategory     = s.selectedCategory;
    const bool categoriesActive = (s.selectedItem < 0);

    // Block all pane navigation while a slider is being edited so that
    // left/right reach the widget instead of being swallowed by the
    // pane logic.
    if (!wasEditingSlider)
    {
        navigatePane(ctx, svc, s.selectedCategory, kCatNavCount, categoriesActive);

        // Item-count for the (possibly newly-selected) category. Computed
        // *after* navigation so navigatePane below uses the right bound.
        const auto itemNFor = [&](int catIdx) -> int
        { return (catIdx == kBackRow) ? 0 : totalItems(kCategories[catIdx]); };

        if (categoriesActive)
        {
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
                s.selectedItem = 0;
            }
        }
        else
        {
            const int curItemN = itemNFor(s.selectedCategory);
            if (curItemN <= 0)
            {
                s.selectedItem = -1;
            }
            else
            {
                navigatePane(ctx, svc, s.selectedItem, curItemN, /*active=*/true);
                if (ctx.input.left)
                    s.selectedItem = -1; // back to category list
            }
        }
    }

    // Re-capture the category *after* navigation. The previous code held
    // a `const CategoryDef&` from before input handling, so when the user
    // clicked or arrowed onto a different tab, the items pane spent one
    // frame rendering — and worse, mouse-activating — the *previous*
    // category's items against the new category's `selectedItem`. That
    // surfaced as "clicking an option in one tab affects another tab".
    const bool         onBackRow = (s.selectedCategory == kBackRow);
    const CategoryDef& cat       = onBackRow ? kCategories[0] : kCategories[s.selectedCategory];
    const int          itemN     = onBackRow ? 0 : totalItems(cat);

    // Snap the item pill (no animation) when the category changes — the
    // visible items list has changed entirely, so a smooth transition would
    // travel through unrelated rows. Also clamp `selectedItem` against the
    // *new* category's item count.
    if (prevCategory != s.selectedCategory)
    {
        if (s.selectedItem >= itemN)
            s.selectedItem = itemN > 0 ? 0 : -1;
        s.itemSelectionY = (s.selectedItem >= 0) ? static_cast<float>(s.selectedItem) * ctx.rowHeight : 0.f;
    }

    // ---- Layout -----------------------------------------------------------
    beginScreen(ctx, screenOrigin(ctx), "OPTIONS");

    const sf::Vec2f leftColTop  = {ctx.origin.x, ctx.cursor.y};
    const sf::Vec2f rightColTop = {ctx.origin.x + 240.f, ctx.cursor.y};

    // ---- Category list (left column) --------------------------------------
    ctx.cursor = ctx.origin = leftColTop;
    animatedPill(ctx, leftColTop, 220.f, s.selectedCategory, s.categorySelectionY, categoriesActive);

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

    // Item selection pill — only relevant once focus has moved here (no
    // pill is drawn at all when the user is still on the category list).
    if (s.selectedItem >= 0 && itemN > 0)
    {
        animatedPill(ctx, rightColTop, 420.f, s.selectedItem, s.itemSelectionY, /*active=*/true);
    }

    int rowIdx = 0;

    for (int i = 0; i < cat.toggleCount; ++i)
    {
        const bool focused = (s.selectedItem == rowIdx);
        bool       v       = cat.toggles[i].get();

        if (toggle(ctx, cat.toggles[i].label, v, focused, 420.f, cat.toggles[i].enabled()))
        {
            cat.toggles[i].set(v);
        }

        ++rowIdx;
    }

    for (int i = 0; i < cat.sliderCount; ++i)
    {
        const bool focused = (s.selectedItem == rowIdx);
        float      v       = cat.sliders[i].get();
        // Only the focused slider sees the shared edit flag; unfocused
        // ones can never enter edit mode anyway, so passing the same
        // reference is safe.
        if (slider(ctx, cat.sliders[i].label, v, cat.sliders[i].min, cat.sliders[i].max, cat.sliders[i].step, focused, s.sliderEditing, 420.f))
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

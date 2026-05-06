// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

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
    bool        (*get)();
    void        (*set)(bool);
};

struct SliderItem
{
    const char* label;
    float       (*get)();
    void        (*set)(float);
    float       min;
    float       max;
    float       step;
};

////////////////////////////////////////////////////////////////////////////////
// GAMEPLAY

constexpr ToggleItem kGameplayToggles[] = {
    {"NO PULSE",          &Config::getNoPulse,          &Config::setNoPulse},
    {"NO ROTATION",       &Config::getNoRotation,       &Config::setNoRotation},
    {"NO BACKGROUND",     &Config::getNoBackground,     &Config::setNoBackground},
    {"BLACK & WHITE",     &Config::getBlackAndWhite,    &Config::setBlackAndWhite},
    {"AUTO RESTART",      &Config::getAutoRestart,      &Config::setAutoRestart},
    {"INVINCIBLE",        &Config::getInvincible,       &Config::setInvincible},
    {"ROTATE TO START",   &Config::getRotateToStart,    &Config::setRotateToStart},
};

constexpr int kGameplayToggleCount = static_cast<int>(sizeof(kGameplayToggles) / sizeof(*kGameplayToggles));

////////////////////////////////////////////////////////////////////////////////
// AUDIO

constexpr ToggleItem kAudioToggles[] = {
    {"NO SOUND",          &Config::getNoSound,          &Config::setNoSound},
    {"NO MUSIC",          &Config::getNoMusic,          &Config::setNoMusic},
    {"MUSIC SPEED SYNC",  &Config::getMusicSpeedDMSync, &Config::setMusicSpeedDMSync},
};
constexpr int kAudioToggleCount = static_cast<int>(sizeof(kAudioToggles) / sizeof(*kAudioToggles));

constexpr SliderItem kAudioSliders[] = {
    {"SOUND VOLUME",  &Config::getSoundVolume,    &Config::setSoundVolume,    0.f,  100.f, 5.f},
    {"MUSIC VOLUME",  &Config::getMusicVolume,    &Config::setMusicVolume,    0.f,  100.f, 5.f},
    {"MUSIC SPEED",   &Config::getMusicSpeedMult, &Config::setMusicSpeedMult, 0.5f, 2.f,   0.05f},
};
constexpr int kAudioSliderCount = static_cast<int>(sizeof(kAudioSliders) / sizeof(*kAudioSliders));

////////////////////////////////////////////////////////////////////////////////
// GRAPHICS

constexpr ToggleItem kGraphicsToggles[] = {
    {"FLASH",                       &Config::getFlash,                         &Config::setFlash},
    {"3D EFFECT",                   &Config::get3D,                            &Config::set3D},
    {"SHADERS",                     &Config::getShaders,                       &Config::setShaders},
    {"SHOW FPS",                    &Config::getShowFPS,                       &Config::setShowFPS},
    {"SHOW TIMER",                  &Config::getShowTimer,                     &Config::setShowTimer},
    {"SHOW STATUS TEXT",            &Config::getShowStatusText,                &Config::setShowStatusText},
    {"SHOW LEVEL INFO",             &Config::getShowLevelInfo,                 &Config::setShowLevelInfo},
    {"SHOW KEY ICONS",              &Config::getShowKeyIcons,                  &Config::setShowKeyIcons},
    {"DARKEN UNEVEN BG CHUNK",      &Config::getDarkenUnevenBackgroundChunk,   &Config::setDarkenUnevenBackgroundChunk},
    {"DRAW TEXT OUTLINES",          &Config::getDrawTextOutlines,              &Config::setDrawTextOutlines},
    {"PLAYER TRAIL",                &Config::getShowPlayerTrail,               &Config::setShowPlayerTrail},
    {"SWAP PARTICLES",              &Config::getShowSwapParticles,             &Config::setShowSwapParticles},
};
constexpr int kGraphicsToggleCount = static_cast<int>(sizeof(kGraphicsToggles) / sizeof(*kGraphicsToggles));

constexpr SliderItem kGraphicsSliders[] = {
    {"CAMERA SHAKE",          &Config::getCameraShakeMultiplier, &Config::setCameraShakeMultiplier, 0.f,  3.f, 0.1f},
    {"ANGLE TILT INTENSITY",  &Config::getAngleTiltIntensity,    &Config::setAngleTiltIntensity,    0.f,  3.f, 0.1f},
    {"TEXT SCALING",          &Config::getTextScaling,           &Config::setTextScaling,           0.5f, 2.f, 0.05f},
};
constexpr int kGraphicsSliderCount = static_cast<int>(sizeof(kGraphicsSliders) / sizeof(*kGraphicsSliders));

////////////////////////////////////////////////////////////////////////////////
// ADVANCED

constexpr ToggleItem kAdvancedToggles[] = {
    {"USE LUA FILE CACHE",     &Config::getUseLuaFileCache,    &Config::setUseLuaFileCache},
    {"DISABLE GAME RENDERING", &Config::getDisableGameRendering, &Config::setDisableGameRendering},
    {"DEBUG MODE",             &Config::getDebug,                &Config::setDebug},
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
    {"GAMEPLAY", kGameplayToggles, kGameplayToggleCount, nullptr,         0},
    {"AUDIO",    kAudioToggles,    kAudioToggleCount,    kAudioSliders,   kAudioSliderCount},
    {"GRAPHICS", kGraphicsToggles, kGraphicsToggleCount, kGraphicsSliders, kGraphicsSliderCount},
    {"ADVANCED", kAdvancedToggles, kAdvancedToggleCount, nullptr,         0},
};
constexpr int kCategoryCount = static_cast<int>(sizeof(kCategories) / sizeof(*kCategories));

[[nodiscard]] int totalItems(const CategoryDef& c) noexcept
{
    return c.toggleCount + c.sliderCount;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////

void drawOptionsScreen(Context& ctx, App& app, Services& /*svc*/)
{
    OptionsScreenState& s = app.options;

    // Clamp category index in case its memory is stale.
    if (s.selectedCategory < 0 || s.selectedCategory >= kCategoryCount)
    {
        s.selectedCategory = 0;
    }

    const CategoryDef& cat   = kCategories[s.selectedCategory];
    const int          itemN = totalItems(cat);

    // ---- Input ------------------------------------------------------------
    if (ctx.input.escape)
    {
        // Escape backs out either the item-pane focus, or the screen entirely.
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

    if (s.selectedItem < 0)
    {
        // Focus on category list.
        if (ctx.input.up)
        {
            s.selectedCategory = (s.selectedCategory + kCategoryCount - 1) % kCategoryCount;
        }
        if (ctx.input.down)
        {
            s.selectedCategory = (s.selectedCategory + 1) % kCategoryCount;
        }
        if (ctx.input.right || ctx.input.enter)
        {
            // Move focus into the items pane.
            s.selectedItem = 0;
        }
    }
    else
    {
        // Focus on item pane.
        if (itemN <= 0)
        {
            s.selectedItem = -1;
        }
        else
        {
            if (ctx.input.up)
            {
                s.selectedItem = (s.selectedItem + itemN - 1) % itemN;
            }
            if (ctx.input.down)
            {
                s.selectedItem = (s.selectedItem + 1) % itemN;
            }
            if (ctx.input.left)
            {
                s.selectedItem = -1; // back to category list
            }
        }
    }

    stepToward(s.openProgress, 1.f, ctx.dt, 6.f);
    stepToward(s.categorySelectionY, static_cast<float>(s.selectedCategory) * ctx.rowHeight, ctx.dt, 18.f);
    if (s.selectedItem >= 0)
    {
        stepToward(s.itemSelectionY, static_cast<float>(s.selectedItem) * ctx.rowHeight, ctx.dt, 18.f);
    }

    // ---- Layout -----------------------------------------------------------
    const sf::Vec2u sz = ctx.target->getSize();

    // Header
    ctx.cursor = ctx.origin = {static_cast<float>(sz.x) * 0.5f - 360.f, static_cast<float>(sz.y) * 0.12f};
    heading(ctx, "OPTIONS");

    const sf::Vec2f leftColTop  = {ctx.origin.x,          ctx.cursor.y};
    const sf::Vec2f rightColTop = {ctx.origin.x + 240.f,  ctx.cursor.y};

    // ---- Category list (left column) --------------------------------------
    ctx.cursor = ctx.origin = leftColTop;
    {
        // Category selection pill behind the focused entry.
        const sf::Vec2f pillPos {ctx.cursor.x - 8.f, ctx.cursor.y + s.categorySelectionY - 4.f};
        const sf::Vec2f pillSize{220.f + 16.f, ctx.rowHeight + 8.f};
        const sf::Color pillFill = (s.selectedItem < 0) ? ctx.colAccent : sf::Color{ctx.colAccent.r, ctx.colAccent.g, ctx.colAccent.b, 80};
        ctx.target->draw(sf::RectangleShapeData{
                             .position  = pillPos,
                             .fillColor = pillFill,
                             .size      = pillSize,
                         },
                         ctx.renderStates);
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

    // ---- Items (right column) ---------------------------------------------
    ctx.cursor = ctx.origin = rightColTop;

    // Item selection pill, only when item pane has focus.
    if (s.selectedItem >= 0 && itemN > 0)
    {
        const sf::Vec2f pillPos {ctx.cursor.x - 8.f, ctx.cursor.y + s.itemSelectionY - 4.f};
        const sf::Vec2f pillSize{420.f + 16.f, ctx.rowHeight + 8.f};
        ctx.target->draw(sf::RectangleShapeData{
                             .position  = pillPos,
                             .fillColor = ctx.colAccent,
                             .size      = pillSize,
                         },
                         ctx.renderStates);
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
        if (slider(ctx, cat.sliders[i].label, v, cat.sliders[i].min, cat.sliders[i].max,
                   cat.sliders[i].step, focused, 420.f))
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

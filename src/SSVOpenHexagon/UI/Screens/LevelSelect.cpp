// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/PackInfo.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/String.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace hg::ui
{

namespace
{

////////////////////////////////////////////////////////////////////////////////
// Helpers

// Case-insensitive substring match. `haystack` is the level metadata, `needle`
// is the user's search input. Empty needle matches everything.
[[nodiscard]] bool matchesSearch(sf::base::StringView haystack, sf::base::StringView needle) noexcept
{
    if (needle.empty()) return true;
    if (haystack.size() < needle.size()) return false;

    auto toLower = [](char c) -> char {
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

[[nodiscard]] bool levelPasses(const LevelData&             ld,
                               sf::base::StringView         search,
                               bool                         favoritesOnly,
                               const ProfileData*           profile)
{
    if (!ld.selectable)
    {
        return false;
    }
    if (favoritesOnly && profile != nullptr && !profile->isLevelFavorite(ld.id))
    {
        return false;
    }
    if (!search.empty())
    {
        const bool nameOk = matchesSearch(ld.name.toStringView(), search);
        const bool authOk = matchesSearch(ld.author.toStringView(), search);
        const bool descOk = matchesSearch(ld.description.toStringView(), search);
        if (!nameOk && !authOk && !descOk)
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] float minDiff(const LevelData& ld) noexcept
{
    if (ld.difficultyMults.empty()) return 1.f;
    float m = ld.difficultyMults[0];
    for (const float v : ld.difficultyMults)
    {
        if (v < m) m = v;
    }
    return m;
}

void rebuildFilteredList(LevelSelectScreenState& s, const HGAssets& assets, const ProfileData* profile)
{
    s.filteredLevelIds.clear();

    const auto search = s.search.toStringView();

    // Iterate packs in priority order, then their levels.
    for (const PackInfo& pi : assets.getSelectablePackInfos())
    {
        const auto& levelIds = const_cast<HGAssets&>(assets).getLevelIdsByPack(pi.id);
        for (const auto& levelId : levelIds)
        {
            const LevelData& ld = assets.getLevelData(levelId);
            if (levelPasses(ld, search, s.favoritesOnly, profile))
            {
                s.filteredLevelIds.emplaceBack(levelId);
            }
        }
    }

    // Sort according to current key. PackPriority is the natural enumeration
    // order from the loop above, so we only re-sort for other keys.
    if (s.sortKey != LevelSortKey::PackPriority)
    {
        const auto getLD = [&assets](const sf::base::String& id) -> const LevelData& {
            return assets.getLevelData(id);
        };

        switch (s.sortKey)
        {
            case LevelSortKey::Name:
                std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(),
                          [&getLD](const auto& a, const auto& b) {
                              return getLD(a).name < getLD(b).name;
                          });
                break;
            case LevelSortKey::Author:
                std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(),
                          [&getLD](const auto& a, const auto& b) {
                              return getLD(a).author < getLD(b).author;
                          });
                break;
            case LevelSortKey::MinDifficulty:
                std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(),
                          [&getLD](const auto& a, const auto& b) {
                              return minDiff(getLD(a)) < minDiff(getLD(b));
                          });
                break;
            case LevelSortKey::PersonalBest:
                if (profile != nullptr)
                {
                    std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(),
                              [&getLD, profile](const auto& a, const auto& b) {
                                  // Use validator at first difficulty for a quick read.
                                  const auto& la = getLD(a);
                                  const auto& lb = getLD(b);
                                  const float aBest = la.difficultyMults.empty() ? 0.f : profile->getScore(la.getValidator(la.difficultyMults[0]));
                                  const float bBest = lb.difficultyMults.empty() ? 0.f : profile->getScore(lb.getValidator(lb.difficultyMults[0]));
                                  return aBest > bBest; // descending
                              });
                }
                break;
            case LevelSortKey::LastPlayed:
                if (profile != nullptr)
                {
                    std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(),
                              [profile](const auto& a, const auto& b) {
                                  return profile->getPerLevelState(a).lastPlayedTs >
                                         profile->getPerLevelState(b).lastPlayedTs;
                              });
                }
                break;
            case LevelSortKey::PlayCount:
                if (profile != nullptr)
                {
                    std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(),
                              [profile](const auto& a, const auto& b) {
                                  return profile->getPerLevelState(a).playCount >
                                         profile->getPerLevelState(b).playCount;
                              });
                }
                break;
            default: break;
        }
    }

    if (s.levelIdx < 0 || s.levelIdx >= static_cast<int>(s.filteredLevelIds.size()))
    {
        s.levelIdx = 0;
    }
    s.cacheStale = false;
}

[[nodiscard]] const char* sortKeyLabel(LevelSortKey k) noexcept
{
    switch (k)
    {
        case LevelSortKey::PackPriority:  return "PACK ORDER";
        case LevelSortKey::Name:          return "NAME";
        case LevelSortKey::Author:        return "AUTHOR";
        case LevelSortKey::MinDifficulty: return "DIFFICULTY";
        case LevelSortKey::PersonalBest:  return "BEST SCORE";
        case LevelSortKey::LastPlayed:    return "LAST PLAYED";
        case LevelSortKey::PlayCount:     return "PLAY COUNT";
    }
    return "?";
}

LevelSortKey nextSortKey(LevelSortKey k) noexcept
{
    constexpr int kCount = 7;
    return static_cast<LevelSortKey>((static_cast<int>(k) + 1) % kCount);
}

} // namespace

////////////////////////////////////////////////////////////////////////////////

void drawLevelSelectScreen(Context& ctx, App& app, Services& svc)
{
    LevelSelectScreenState& s = app.levelSelect;

    // ---- No backend? render a placeholder and bail. ------------------------
    if (svc.assets == nullptr)
    {
        ctx.cursor = ctx.origin = {static_cast<float>(ctx.target->getSize().x) * 0.5f - 200.f,
                                    static_cast<float>(ctx.target->getSize().y) * 0.30f};
        heading(ctx, "LEVEL SELECT");
        label(ctx, "(assets not available)");
        if (button(ctx, "BACK", true))
        {
            goBack(app);
            return;
        }
        return;
    }

    HGAssets&          assets  = *svc.assets;
    const ProfileData* profile = svc.currentProfile;

    // ---- Invalidate cache if pack list changed ----------------------------
    if (assets.packListVersion() != s.cachedPackListVersion)
    {
        s.cachedPackListVersion = assets.packListVersion();
        s.cacheStale            = true;
    }
    if (s.cacheStale)
    {
        rebuildFilteredList(s, assets, profile);
    }

    const int n = static_cast<int>(s.filteredLevelIds.size());

    // ---- Input ------------------------------------------------------------
    if (ctx.input.escape)
    {
        goBack(app);
        return;
    }

    if (n > 0)
    {
        if (ctx.input.up)   s.levelIdx = (s.levelIdx + n - 1) % n;
        if (ctx.input.down) s.levelIdx = (s.levelIdx + 1) % n;

        const LevelData& currentLevel = assets.getLevelData(s.filteredLevelIds[s.levelIdx]);
        const int        diffN        = static_cast<int>(currentLevel.difficultyMults.size());
        if (diffN > 0)
        {
            if (s.difficultyIdx < 0 || s.difficultyIdx >= diffN)
            {
                s.difficultyIdx = 0;
            }
            if (ctx.input.left)  s.difficultyIdx = (s.difficultyIdx + diffN - 1) % diffN;
            if (ctx.input.right) s.difficultyIdx = (s.difficultyIdx + 1) % diffN;
        }

        if (ctx.input.enter)
        {
            if (svc.onStartLevel)
            {
                const float dm = (diffN > 0) ? currentLevel.difficultyMults[s.difficultyIdx] : 1.f;
                svc.onStartLevel(currentLevel.id, dm);
                return;
            }
        }
    }

    // ---- Animation step ---------------------------------------------------
    stepToward(s.openProgress, 1.f, ctx.dt, 6.f);
    stepToward(s.selectionY, static_cast<float>(s.levelIdx) * ctx.rowHeight, ctx.dt, 18.f);

    // ---- Layout -----------------------------------------------------------
    const sf::Vec2u sz   = ctx.target->getSize();
    const float     left = static_cast<float>(sz.x) * 0.06f;
    const float     top  = static_cast<float>(sz.y) * 0.10f;

    ctx.cursor = ctx.origin = {left, top};
    heading(ctx, "LEVEL SELECT");

    // ---- Toolbar ----------------------------------------------------------
    {
        // Sort indicator (cycle with keyboard "S" handled separately;
        // here it's a button you click to advance).
        char sortBuf[64] = {};
        std::snprintf(sortBuf, sizeof(sortBuf), "SORT: %s", sortKeyLabel(s.sortKey));
        if (button(ctx, sortBuf, false, 240.f))
        {
            s.sortKey    = nextSortKey(s.sortKey);
            s.cacheStale = true;
        }

        // Favorites toggle.
        ctx.cursor.x = left + 250.f;
        ctx.cursor.y -= ctx.rowHeight;
        bool favOnly = s.favoritesOnly;
        if (toggle(ctx, "FAVORITES ONLY", favOnly, false, 240.f))
        {
            s.favoritesOnly = favOnly;
            s.cacheStale    = true;
        }

        // Search field.
        ctx.cursor.x = left + 500.f;
        ctx.cursor.y -= ctx.rowHeight;
        bool       searchSubmitted = false;
        const auto searchSizeBefore = s.search.size();
        if (textField(ctx, "SEARCH", s.search, true, searchSubmitted, 360.f))
        {
            s.cacheStale = true;
        }
        (void)searchSubmitted;
        (void)searchSizeBefore;

        ctx.cursor.x = left;
        newLine(ctx, 12.f);
    }

    // Save the row where the two-pane content starts.
    const float listLeft  = left;
    const float listTop   = ctx.cursor.y;
    const float detailsLeft = left + 460.f;

    // ---- Level list (left pane) -------------------------------------------
    {
        ctx.cursor = ctx.origin = {listLeft, listTop};

        // Selection pill.
        if (n > 0)
        {
            const sf::Vec2f pillPos {ctx.cursor.x - 8.f, ctx.cursor.y + s.selectionY - 4.f};
            const sf::Vec2f pillSize{420.f + 16.f, ctx.rowHeight + 8.f};
            ctx.target->draw(sf::RectangleShapeData{
                                 .position  = pillPos,
                                 .fillColor = ctx.colAccent,
                                 .size      = pillSize,
                             },
                             ctx.renderStates);
        }

        if (n == 0)
        {
            label(ctx, "(no levels match)");
        }
        else
        {
            // Cap the visible window so the list doesn't run off the bottom.
            const int   maxVisible = 14;
            int         start      = 0;
            if (s.levelIdx >= maxVisible)
            {
                start = s.levelIdx - maxVisible + 1;
            }
            const int end = std::min(n, start + maxVisible);

            for (int i = start; i < end; ++i)
            {
                const LevelData& ld = assets.getLevelData(s.filteredLevelIds[i]);

                // "★" prefix if favorited
                const bool isFav = (profile != nullptr) && profile->isLevelFavorite(ld.id);

                char rowText[160] = {};
                std::snprintf(rowText, sizeof(rowText), "%s %.*s",
                              isFav ? "*" : " ",
                              static_cast<int>(ld.name.size()),
                              ld.name.cStr());

                if (button(ctx, rowText, i == s.levelIdx, 420.f))
                {
                    s.levelIdx = i;
                }
            }
        }
    }

    // ---- Details pane (right) ---------------------------------------------
    {
        ctx.cursor = ctx.origin = {detailsLeft, listTop};

        if (n == 0)
        {
            label(ctx, "");
            return;
        }

        const LevelData& cur = assets.getLevelData(s.filteredLevelIds[s.levelIdx]);

        // Name (highlight color)
        ctx.target->draw(*ctx.font,
                         sf::TextData{
                             .position      = ctx.cursor,
                             .string        = sf::UnicodeString{cur.name.cStr()},
                             .characterSize = static_cast<unsigned int>(ctx.fontSize * 1.3f),
                             .fillColor     = ctx.colHighlight,
                         },
                         ctx.renderStates);
        ctx.cursor.y += ctx.rowHeight + 6.f;

        // Author
        char authBuf[160] = {};
        std::snprintf(authBuf, sizeof(authBuf), "BY  %.*s", static_cast<int>(cur.author.size()), cur.author.cStr());
        label(ctx, authBuf);

        // Description (capped to fit)
        if (!cur.description.empty())
        {
            char descBuf[256] = {};
            std::snprintf(descBuf, sizeof(descBuf), "%.*s",
                          static_cast<int>(std::min<sf::base::SizeT>(cur.description.size(), 220)),
                          cur.description.cStr());
            label(ctx, descBuf);
        }

        newLine(ctx, 4.f);
        separator(ctx);
        newLine(ctx, 4.f);

        // Difficulty selector
        if (!cur.difficultyMults.empty())
        {
            if (s.difficultyIdx < 0 || s.difficultyIdx >= static_cast<int>(cur.difficultyMults.size()))
            {
                s.difficultyIdx = 0;
            }
            char diffBuf[64] = {};
            std::snprintf(diffBuf, sizeof(diffBuf), "DIFFICULTY:  < %.2f >",
                          static_cast<double>(cur.difficultyMults[s.difficultyIdx]));
            label(ctx, diffBuf);
        }

        // Best score
        if (profile != nullptr && !cur.difficultyMults.empty())
        {
            const float dm   = cur.difficultyMults[s.difficultyIdx];
            const float best = profile->getScore(cur.getValidator(dm));
            char        bestBuf[64] = {};
            if (best > 0.f)
            {
                std::snprintf(bestBuf, sizeof(bestBuf), "BEST:  %.2fs", static_cast<double>(best));
            }
            else
            {
                std::snprintf(bestBuf, sizeof(bestBuf), "BEST:  N/A");
            }
            label(ctx, bestBuf);
        }

        // Music / pack info
        if (!cur.musicId.empty())
        {
            char musicBuf[160] = {};
            std::snprintf(musicBuf, sizeof(musicBuf), "MUSIC:  %.*s",
                          static_cast<int>(cur.musicId.size()), cur.musicId.cStr());
            label(ctx, musicBuf);
        }
        if (!cur.packId.empty())
        {
            char packBuf[160] = {};
            std::snprintf(packBuf, sizeof(packBuf), "PACK:   %.*s",
                          static_cast<int>(cur.packId.size()), cur.packId.cStr());
            label(ctx, packBuf);
        }

        newLine(ctx, 8.f);

        // Favorite toggle (in the details pane, button-style)
        if (profile != nullptr)
        {
            const bool isFav = profile->isLevelFavorite(cur.id);
            const char* favLabel = isFav ? "* UNFAVORITE" : "  FAVORITE";
            if (button(ctx, favLabel, false, 240.f))
            {
                ProfileData& mutProfile = const_cast<ProfileData&>(*profile);
                if (isFav) mutProfile.removeFavoriteLevel(cur.id);
                else       mutProfile.addFavoriteLevel(cur.id);
                if (s.favoritesOnly) s.cacheStale = true;
            }
        }

        if (button(ctx, "PLAY", false, 240.f))
        {
            if (svc.onStartLevel)
            {
                const float dm = cur.difficultyMults.empty() ? 1.f : cur.difficultyMults[s.difficultyIdx];
                svc.onStartLevel(cur.id, dm);
                return;
            }
        }

        if (button(ctx, "BACK", false, 240.f))
        {
            goBack(app);
            return;
        }
    }
}

} // namespace hg::ui

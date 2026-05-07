// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/MusicData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/PackInfo.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/Texture.hpp"

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

[[nodiscard]] bool levelPasses(const LevelData& ld, sf::base::StringView search, bool favoritesOnly, const ProfileData* profile)
{
    if (!ld.selectable)
        return false;
    if (favoritesOnly && profile != nullptr && !profile->isLevelFavorite(ld.id))
        return false;
    if (!search.empty() && !containsCI(ld.name.toStringView(), search) &&
        !containsCI(ld.author.toStringView(), search) && !containsCI(ld.description.toStringView(), search))
    {
        return false;
    }
    return true;
}

[[nodiscard]] float minDiff(const LevelData& ld) noexcept
{
    if (ld.difficultyMults.empty())
        return 1.f;
    float m = ld.difficultyMults[0];
    for (const float v : ld.difficultyMults)
    {
        if (v < m)
            m = v;
    }
    return m;
}

void rebuildFilteredList(LevelSelectScreenState& s, const HGAssets& assets, const ProfileData* profile)
{
    s.filteredLevelIds.clear();
    s.filteredDisplayRows.clear();

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
        const auto getLD = [&assets](const sf::base::String& id) -> const LevelData& { return assets.getLevelData(id); };

        switch (s.sortKey)
        {
            case LevelSortKey::Name:
                std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(), [&getLD](const auto& a, const auto& b) {
                    return getLD(a).name < getLD(b).name;
                });
                break;
            case LevelSortKey::Author:
                std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(), [&getLD](const auto& a, const auto& b) {
                    return getLD(a).author < getLD(b).author;
                });
                break;
            case LevelSortKey::MinDifficulty:
                std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(), [&getLD](const auto& a, const auto& b) {
                    return minDiff(getLD(a)) < minDiff(getLD(b));
                });
                break;
            case LevelSortKey::PersonalBest:
                if (profile != nullptr)
                {
                    std::sort(s.filteredLevelIds.begin(),
                              s.filteredLevelIds.end(),
                              [&getLD, profile](const auto& a, const auto& b)
                    {
                        // Use validator at first difficulty for a quick read.
                        const auto& la    = getLD(a);
                        const auto& lb    = getLD(b);
                        const float aBest = la.difficultyMults.empty()
                                                ? 0.f
                                                : profile->getScore(la.getValidator(la.difficultyMults[0]));
                        const float bBest = lb.difficultyMults.empty()
                                                ? 0.f
                                                : profile->getScore(lb.getValidator(lb.difficultyMults[0]));
                        return aBest > bBest; // descending
                    });
                }
                break;
            case LevelSortKey::LastPlayed:
                if (profile != nullptr)
                {
                    std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(), [profile](const auto& a, const auto& b) {
                        return profile->getPerLevelState(a).lastPlayedTs > profile->getPerLevelState(b).lastPlayedTs;
                    });
                }
                break;
            case LevelSortKey::PlayCount:
                if (profile != nullptr)
                {
                    std::sort(s.filteredLevelIds.begin(), s.filteredLevelIds.end(), [profile](const auto& a, const auto& b) {
                        return profile->getPerLevelState(a).playCount > profile->getPerLevelState(b).playCount;
                    });
                }
                break;
            default:
                break;
        }
    }

    if (s.levelIdx < 0 || s.levelIdx >= static_cast<int>(s.filteredLevelIds.size()))
    {
        s.levelIdx = 0;
    }

    // Compute display rows. Pack grouping (with header rows above each
    // group) only applies to the default sort, where levels are already
    // adjacent within their pack. Other sort orders intermix packs, so a
    // header would be ambiguous -- use a flat list there.
    s.filteredDisplayRows.clear();
    s.filteredDisplayRows.reserve(s.filteredLevelIds.size());
    if (s.sortKey == LevelSortKey::PackPriority)
    {
        int                  row = 0;
        sf::base::StringView prevPack;
        for (const auto& id : s.filteredLevelIds)
        {
            const auto pid = assets.getLevelData(id).packId.toStringView();
            if (pid != prevPack)
            {
                ++row;
                prevPack = pid;
            } // reserve a header row
            s.filteredDisplayRows.emplaceBack(row);
            ++row;
        }
    }
    else
    {
        for (sf::base::SizeT i = 0; i < s.filteredLevelIds.size(); ++i)
        {
            s.filteredDisplayRows.emplaceBack(static_cast<int>(i));
        }
    }
    s.cacheStale = false;
}

[[nodiscard]] const char* sortKeyLabel(LevelSortKey k) noexcept
{
    switch (k)
    {
        case LevelSortKey::PackPriority:
            return "PACK ORDER";
        case LevelSortKey::Name:
            return "NAME";
        case LevelSortKey::Author:
            return "AUTHOR";
        case LevelSortKey::MinDifficulty:
            return "DIFFICULTY";
        case LevelSortKey::PersonalBest:
            return "BEST SCORE";
        case LevelSortKey::LastPlayed:
            return "LAST PLAYED";
        case LevelSortKey::PlayCount:
            return "PLAY COUNT";
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
        beginScreen(ctx, screenOrigin(ctx), "LEVEL SELECT");
        label(ctx, "(assets not available)");
        if (button(ctx, "BACK", true))
            goBack(app);
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
    if (handleEscape(ctx, app))
        return;

    // Action rows in the right pane (FAVORITE conditional on a profile).
    enum class ActionRow
    {
        Favorite,
        Play,
        Difficulty,
        Back
    };
    const bool hasProfile  = (profile != nullptr);
    const int  actionCount = hasProfile ? 4 : 3;

    // Pack column metadata (used by both input + render).
    const auto& packInfos = assets.getSelectablePackInfos();
    const int   packCount = static_cast<int>(packInfos.size());

    int activePackIdx = 0;
    if (n > 0)
    {
        const auto activePackId = assets.getLevelData(s.filteredLevelIds[s.levelIdx]).packId.toStringView();
        for (int i = 0; i < packCount; ++i)
        {
            if (packInfos[i].id.toStringView() == activePackId)
            {
                activePackIdx = i;
                break;
            }
        }
    }

    // ---- Pane switching (left / right) ------------------------------------
    // Pane indices: 0 = Packs, 1 = Levels, 2 = Actions, 3 = Leaderboard.
    // Non-wrapping left/right via the shared helper, matching Options +
    // Workshop.
    using Pane     = LevelSelectScreenState::Pane;
    int activePane = static_cast<int>(s.pane);
    paneSwitchLeftRight(ctx, svc, activePane, 4);
    s.pane = static_cast<Pane>(activePane);

    // ---- Navigation within the current pane -------------------------------
    {
        int packIdx = activePackIdx;
        if (navigatePane(ctx, svc, packIdx, packCount, s.pane == Pane::Packs))
        {
            // Jump the level cursor to the first level of the newly-focused
            // pack (if any), and scroll the level list so it sits at the top.
            for (int j = 0; j < n; ++j)
            {
                if (assets.getLevelData(s.filteredLevelIds[j]).packId.toStringView() ==
                    packInfos[packIdx].id.toStringView())
                {
                    s.levelIdx    = j;
                    s.scrollStart = j;
                    break;
                }
            }
        }
    }
    navigatePane(ctx, svc, s.levelIdx, n, s.pane == Pane::Levels);
    navigatePane(ctx, svc, s.actionIdx, actionCount, s.pane == Pane::Actions);
    if (s.actionIdx < 0 || s.actionIdx >= actionCount)
        s.actionIdx = 0;

    // Leaderboard pane navigation. Row count is dynamic -- it's the
    // size of the currently-published score list, or 0 when the list
    // isn't ready (offline / loading / unsupported). When 0,
    // `navigatePane` clamps the index to 0; the column just shows the
    // empty-state message. The actual rendering happens in the
    // details-pane block below.
    {
        const int leaderboardCount = (svc.leaderboardScores != nullptr) ? static_cast<int>(svc.leaderboardScores->size()) : 0;
        navigatePane(ctx, svc, s.leaderboardIdx, leaderboardCount, s.pane == Pane::Leaderboard);
        if (s.leaderboardIdx < 0)
            s.leaderboardIdx = 0;
        if (leaderboardCount > 0 && s.leaderboardIdx >= leaderboardCount)
            s.leaderboardIdx = leaderboardCount - 1;
    }

    if (s.pane == Pane::Levels && n > 0 && ctx.input.enter && svc.onStartLevel)
    {
        const LevelData& cur = assets.getLevelData(s.filteredLevelIds[s.levelIdx]);
        const int        dN  = static_cast<int>(cur.difficultyMults.size());
        const float      dm  = (dN > 0) ? cur.difficultyMults[s.difficultyIdx] : 1.f;
        svc.onStartLevel(s.filteredLevelIds[s.levelIdx], dm);
        return;
    }

    // Enter on a leaderboard row: ask the host for that replay. Reply
    // arrives asynchronously via `EReceivedReplay` -> `fnHGWatchReplay`.
    // The host re-derives (level, difficulty) from its
    // `currentLeaderboardValidator`, which our `onRequestLeaderboard`
    // poll above keeps current -- so we only need to pass the row's
    // timestamp.
    if (s.pane == Pane::Leaderboard && n > 0 && ctx.input.enter && svc.onWatchReplay &&
        svc.leaderboardScores != nullptr && !svc.leaderboardScores->empty())
    {
        const int           idx = std::clamp(s.leaderboardIdx, 0, static_cast<int>(svc.leaderboardScores->size()) - 1);
        const sf::base::U64 ts  = (*svc.leaderboardScores)[static_cast<sf::base::SizeT>(idx)].scoreTimestamp;

        svc.onWatchReplay(ts);
        return;
    }

    // Auto-scroll the level list so `levelIdx` stays inside the window.
    // `kMaxVisible` is measured in *display rows* -- pack-group header rows
    // count too, otherwise the list visually grows / shrinks every time a
    // header scrolls in or out of view.
    constexpr int kMaxVisible = 14;

    // Compute the windowed end (exclusive) for a candidate `start`,
    // counting both level rows and any pack headers that get drawn
    // alongside them. Used both for auto-scroll and for the render loop.
    const auto getPackOf  = [&](int i) { return assets.getLevelData(s.filteredLevelIds[i]).packId.toStringView(); };
    const bool grouping   = (s.sortKey == LevelSortKey::PackPriority);
    const auto computeEnd = [&](int start)
    {
        if (n <= 0)
            return 0;
        int end  = start;
        int used = (grouping && (start == 0 || getPackOf(start) != getPackOf(start - 1))) ? 1 : 0;
        while (end < n)
        {
            int extra = 1;
            if (grouping && end > start && getPackOf(end) != getPackOf(end - 1))
                extra += 1; // pack header inserted before this level
            if (used + extra > kMaxVisible)
                break;
            used += extra;
            ++end;
        }
        return end;
    };

    if (n > 0)
    {
        if (s.levelIdx < s.scrollStart)
        {
            s.scrollStart = s.levelIdx;
        }
        // Push scrollStart forward until levelIdx fits inside the window.
        // The display-row count varies with pack boundaries, so the cheap
        // "subtract kMaxVisible" formula doesn't work -- we walk instead.
        while (s.scrollStart < s.levelIdx && computeEnd(s.scrollStart) <= s.levelIdx)
        {
            ++s.scrollStart;
        }
        if (s.scrollStart > n - 1)
            s.scrollStart = std::max(0, n - 1);
        if (s.scrollStart < 0)
            s.scrollStart = 0;
    }
    constexpr int kMaxPackVisible = 14;
    if (packCount > 0)
    {
        if (activePackIdx < s.packScrollStart)
        {
            s.packScrollStart = activePackIdx;
        }
        else if (activePackIdx >= s.packScrollStart + kMaxPackVisible)
        {
            s.packScrollStart = activePackIdx - kMaxPackVisible + 1;
        }
        if (s.packScrollStart < 0)
            s.packScrollStart = 0;
    }

    // ---- Preview hook -----------------------------------------------------
    // Notify the host whenever the *level under the cursor* changes so the
    // backdrop and preview track the selection. Tracking by id (instead of
    // by `levelIdx`) is what makes the preview refresh after the filter
    // shifts the same index onto a different level.
    if (n > 0 && svc.onPreviewLevel)
    {
        const auto& curId = s.filteredLevelIds[s.levelIdx];
        if (curId != s.lastPreviewedId)
        {
            svc.onPreviewLevel(curId);
            s.lastPreviewedId = curId;
        }
    }

    // ---- Layout -----------------------------------------------------------
    // (`selectionY`/`packSelectionY` animate later, once we know where the
    //  selected row will actually be drawn within the visible window.)
    const sf::Vec2f origin = screenOrigin(ctx);
    const float     left   = origin.x;

    beginScreen(ctx, origin, "LEVEL SELECT");

    // ---- Toolbar ----------------------------------------------------------
    {
        // Sort indicator (cycle with keyboard "S" handled separately;
        // here it's a button you click to advance).
        char sortBuf[64];
        std::snprintf(sortBuf, sizeof(sortBuf), "SORT: %s", sortKeyLabel(s.sortKey));
        if (button(ctx, sortBuf, false, 240.f))
        {
            s.sortKey    = nextSortKey(s.sortKey);
            s.cacheStale = true;
        }

        // Favorites toggle. Wider than other widgets to leave room for the
        // long label on the left and the `ON/OFF` marker on the right.
        ctx.cursor.x = left + 250.f;
        ctx.cursor.y -= ctx.rowHeight;
        bool favOnly = s.favoritesOnly;
        if (toggle(ctx, "FAVORITES ONLY", favOnly, false, 320.f))
        {
            s.favoritesOnly = favOnly;
            s.cacheStale    = true;
        }

        // Search field.
        ctx.cursor.x = left + 590.f;
        ctx.cursor.y -= ctx.rowHeight;
        bool searchSubmitted = false;
        if (textField(ctx, "SEARCH", s.search, true, searchSubmitted, 360.f))
        {
            s.cacheStale = true;
        }
        (void)searchSubmitted;

        // 1-based "[ idx / total ]" counter, sitting at the right end of
        // the toolbar where the heading used to carry it.
        if (n > 0)
        {
            char countBuf[32];
            std::snprintf(countBuf, sizeof(countBuf), "[ %d / %d ]", s.levelIdx + 1, n);
            ctx.cursor.x = left + 970.f;
            ctx.cursor.y -= ctx.rowHeight;
            label(ctx, countBuf, 180.f);
        }

        ctx.cursor.x = left;
        newLine(ctx, 12.f);
    }

    // Three-column layout: PACKS | LEVELS | DETAILS.
    // Widths kept generous in the details column so long level names fit.
    constexpr float kPacksW     = 280.f;
    constexpr float kListW      = 420.f;
    constexpr float kDetailsW   = 540.f;
    const float     packsLeft   = left;
    const float     listLeft    = left + kPacksW + 20.f;
    const float     listTop     = ctx.cursor.y;
    const float     detailsLeft = listLeft + kListW + 20.f;

    // ---- Pack column (far left) -------------------------------------------
    {
        const int packStart = s.packScrollStart;
        const int packEnd   = std::min(packCount, packStart + kMaxPackVisible);

        // Header label above the list -- offsets every pack row down by
        // one row so the pill animation also has to start below it.
        const float kPacksHeaderH   = ctx.rowHeight + 4.f;
        const float packsContentTop = listTop + kPacksHeaderH;

        if (packCount > 0)
        {
            animatedPill(ctx,
                         {packsLeft, packsContentTop},
                         kPacksW,
                         activePackIdx - packStart,
                         s.packSelectionY,
                         s.pane == Pane::Packs);
        }

        ctx.cursor = ctx.origin = {packsLeft, listTop};
        // Header rendered in the accent color (gradient sentinel -- the
        // post-process shader paints it with the animated noise gradient).
        const sf::Color textBefore = ctx.colText;
        ctx.colText                = ctx.colAccent;
        label(ctx, "[ PACKS ]", kPacksW);
        ctx.colText            = textBefore;
        ctx.cursor.x           = packsLeft;
        ctx.cursor.y           = packsContentTop;
        const auto packLabelOf = [&](int i, char* buf, sf::base::SizeT bufSize)
        {
            const auto& pi       = packInfos[i];
            const auto& packData = const_cast<HGAssets&>(assets).getPackData(pi.id);
            const auto& pname    = packData.name.empty() ? pi.id : packData.name;
            std::snprintf(buf, bufSize, "%.*s", static_cast<int>(pname.size()), pname.cStr());
        };

        for (int i = packStart; i < packEnd; ++i)
        {
            char buf[160];
            packLabelOf(i, buf, sizeof(buf));

            if (button(ctx, buf, i == activePackIdx, kPacksW))
            {
                // Click jumps the level cursor to the first level of this
                // pack that passes the current filter and scrolls the level
                // list to the top so the pack is visible from row 0.
                for (int j = 0; j < n; ++j)
                {
                    if (assets.getLevelData(s.filteredLevelIds[j]).packId.toStringView() == packInfos[i].id.toStringView())
                    {
                        s.levelIdx    = j;
                        s.scrollStart = j;
                        break;
                    }
                }
                s.pane = Pane::Packs;
            }
        }

        // Faded peek of the next pack below the visible window -- visual
        // hint that the list scrolls.
        if (packEnd < packCount)
        {
            char buf[160];
            packLabelOf(packEnd, buf, sizeof(buf));
            drawFadedPeekRow(ctx, buf, kPacksW);
        }
    }

    // ---- Level list (middle pane) -----------------------------------------
    if (n == 0)
    {
        ctx.cursor = ctx.origin = {listLeft, listTop};
        label(ctx, "(no levels match)");
    }
    else
    {
        // Persistent windowing -- scroll position only changes when the
        // selection leaves the visible range or when an explicit jump
        // (e.g. pack click) repositions it. `end` is computed via
        // `computeEnd` so pack-header rows count toward the visible
        // budget; otherwise inserting a header would push the bottom
        // level off-screen.
        const int start = s.scrollStart;
        const int end   = computeEnd(start);

        // Helper: pack id of the level at index `i`. Used in two places.
        const auto packOf = [&](int i) { return assets.getLevelData(s.filteredLevelIds[i]).packId.toStringView(); };

        // The pill's visible-relative y is derived from the cached
        // `filteredDisplayRows` (built at rebuild time, accounting for
        // header rows). When the window starts at a pack boundary we draw
        // an extra header row at the top, which shifts following rows
        // down by one -- accounted for by `topHeader`.
        // The pill's index isn't `levelIdx` directly -- pack-group header
        // rows shift the visual rows down. `rel` is the display-row index
        // of the focused level inside the visible window; pass it as the
        // pill helper's `idx` so the animation lands on the right row.
        const bool topHeader = grouping && (start == 0 || packOf(start) != packOf(start - 1));
        const int  rel       = s.filteredDisplayRows[s.levelIdx] - s.filteredDisplayRows[start] + (topHeader ? 1 : 0);
        animatedPill(ctx, {listLeft, listTop}, kListW, rel, s.selectionY, s.pane == Pane::Levels);

        // Render header rows + level buttons.
        ctx.cursor = ctx.origin       = {listLeft, listTop};
        sf::base::StringView prevPack = (grouping && start > 0) ? packOf(start - 1) : sf::base::StringView{};

        for (int i = start; i < end; ++i)
        {
            const LevelData& ld = assets.getLevelData(s.filteredLevelIds[i]);

            if (grouping)
            {
                const auto pid = ld.packId.toStringView();
                if (pid != prevPack)
                {
                    const auto& packData = const_cast<HGAssets&>(assets).getPackData(ld.packId);
                    const auto& pname    = packData.name.empty() ? ld.packId : packData.name;
                    char        headerBuf[160];
                    std::snprintf(headerBuf, sizeof(headerBuf), "[ %.*s ]", static_cast<int>(pname.size()), pname.cStr());
                    truncateToFit(headerBuf, kListW - 24.f, ctx.fontSize * 0.95f);
                    // Pack header: row backdrop + accent-tinted text. Manual
                    // because we want the smaller font size and accent color.
                    ctx.target->draw(
                        sf::RectangleShapeData{
                            .position  = ctx.cursor,
                            .fillColor = ctx.colRow,
                            .size      = {kListW, ctx.rowHeight},
                        },
                        ctx.renderStates);
                    ctx.target->draw(*ctx.font,
                                     sf::TextData{
                                         .position      = ctx.cursor + sf::Vec2f{12.f, 4.f},
                                         .string        = sf::UnicodeString{headerBuf},
                                         .characterSize = static_cast<unsigned int>(ctx.fontSize * 0.95f),
                                         .fillColor     = ctx.colAccent,
                                     },
                                     ctx.renderStates);
                    ctx.cursor.y += ctx.rowHeight;
                    prevPack = pid;
                }
            }

            const bool isFav = (profile != nullptr) && profile->isLevelFavorite(ld.id);
            char       rowText[160];
            std::snprintf(rowText,
                          sizeof(rowText),
                          "%s %.*s",
                          isFav ? "*" : " ",
                          static_cast<int>(ld.name.size()),
                          ld.name.cStr());

            if (button(ctx, rowText, i == s.levelIdx, kListW))
            {
                s.levelIdx = i;
                s.pane     = Pane::Levels;
            }
        }

        // Faded peek of the next level below the visible window -- visual
        // hint that the list scrolls. Skips pack-header rendering for
        // simplicity; the user just needs to know "more below".
        if (end < n)
        {
            const LevelData& nx = assets.getLevelData(s.filteredLevelIds[end]);
            char             buf[160];
            std::snprintf(buf, sizeof(buf), "  %.*s", static_cast<int>(nx.name.size()), nx.name.cStr());
            drawFadedPeekRow(ctx, buf, kListW);
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

        // Title row: row backdrop + larger highlight-colored title text.
        // Routed through `text()` so the case-folding sanitizer
        // upper-cases level names (which arrive in author casing).
        const float titleSize   = ctx.fontSize * 1.3f;
        const float titleHeight = titleSize + 8.f;
        ctx.target->draw(
            sf::RectangleShapeData{
                .position  = ctx.cursor,
                .fillColor = ctx.colRow,
                .size      = {kDetailsW, titleHeight},
            },
            ctx.renderStates);
        char titleBuf[160];
        std::snprintf(titleBuf, sizeof(titleBuf), "%.*s", static_cast<int>(cur.name.size()), cur.name.cStr());
        truncateToFit(titleBuf, kDetailsW - 24.f, titleSize);
        text(ctx, ctx.cursor + sf::Vec2f{12.f, 0.f}, titleBuf, titleSize);
        ctx.cursor.y += titleHeight + 6.f;

        // Live preview of the selected level -- sampled from the off-screen
        // RenderTexture the host paints with `hgPreview`. Occupies the
        // left side of the details column; the action buttons render to
        // its right.
        constexpr float kPreviewW   = 320.f;
        constexpr float kPreviewH   = kPreviewW * (9.f / 16.f); // 180.f
        constexpr float kFrameInset = 6.f;                      // border thickness around the sprite

        const sf::Vec2f previewTopLeft = ctx.cursor;

        // Eased-in/out alpha so the preview doesn't pop visible/invisible
        // when navigating between levels with and without a live preview.
        const float previewTarget = (svc.previewTexture != nullptr) ? 1.f : 0.f;
        stepToward(s.previewAlpha, previewTarget, ctx.dt, 6.f);

        if (s.previewAlpha > 0.f)
        {
            // Combined alpha: per-preview ease (so it ramps when the
            // texture appears/disappears) times the screen-level fade
            // the dispatcher injects during transitions. `colRow` is
            // already faded by the dispatcher, but the sprite's color
            // and the inset frame are hardcoded values that need this
            // multiplier explicitly.
            const float combinedAlpha = s.previewAlpha * ctx.screenAlpha;
            const auto  withAlpha     = [&](sf::Color c)
            { return sf::Color{c.r, c.g, c.b, static_cast<sf::base::U8>(static_cast<float>(c.a) * s.previewAlpha)}; };
            const auto withCombinedAlpha = [&](sf::Color c)
            { return sf::Color{c.r, c.g, c.b, static_cast<sf::base::U8>(static_cast<float>(c.a) * combinedAlpha)}; };

            // Draw the preview backdrop + sprite straight to the window,
            // bypassing the UI composite texture. The accent-gradient
            // shader runs over the composite and would remap any
            // magenta-saturated pixel in a level's preview (e.g. styles
            // that use pink/magenta). Drawing direct to the window skips
            // that pass; the UI composite is alpha-blended on top
            // afterwards, so any text/pills sitting over the preview
            // still overlay correctly. Falls back to `ctx.target` when
            // the host doesn't expose a raw target (headless).
            sf::RenderTarget* const previewTarget = (svc.rawTarget != nullptr) ? svc.rawTarget : ctx.target;

            // Black backdrop framing -- `ctx.colRow` is opaque black in the
            // current theme, and we shrink the rendered sprite by
            // `kFrameInset` on every side so the bg shows through as a
            // visible border.
            previewTarget->draw(
                sf::RectangleShapeData{
                    .position  = previewTopLeft,
                    .fillColor = withAlpha(ctx.colRow),
                    .size      = {kPreviewW, kPreviewH},
                },
                ctx.renderStates);

            if (svc.previewTexture != nullptr)
            {
                const sf::Texture& tex     = svc.previewTexture->getTexture();
                const sf::Vec2u    texSize = tex.getSize();

                if (texSize.x > 0 && texSize.y > 0)
                {
                    const float spriteW = kPreviewW - kFrameInset * 2.f;
                    const float spriteH = kPreviewH - kFrameInset * 2.f;
                    previewTarget->draw(
                        sf::Sprite{
                            .position = {previewTopLeft.x + kFrameInset, previewTopLeft.y + kFrameInset},
                            .scale = {spriteW / static_cast<float>(texSize.x), spriteH / static_cast<float>(texSize.y)},
                            .textureRect = {{0.f, 0.f}, {static_cast<float>(texSize.x), static_cast<float>(texSize.y)}},
                            .color       = withCombinedAlpha(sf::Color::White),
                        },
                        sf::RenderStates{.transform = ctx.renderStates.transform,
                                         .view      = ctx.renderStates.view,
                                         .texture   = &tex});
                }
            }
        }

        // ---- Action pane (FAVORITE / PLAY / DIFFICULTY / BACK) -----------
        // Sits to the right of the preview, sharing the same row baseline.
        // Each row is focusable via up/down when this pane has focus, and
        // mouse-clickable always. The pill behind the focused row uses a
        // desaturated accent when focus is elsewhere.
        constexpr float kColumnGap = 12.f;
        const float     kActionW   = kDetailsW - kPreviewW - kColumnGap;

        // Where metadata resumes below both columns.
        const sf::Vec2f metadataCursor{detailsLeft, previewTopLeft.y + kPreviewH + 8.f};

        // Position the action column at the top of the preview row.
        ctx.cursor = ctx.origin = {previewTopLeft.x + kPreviewW + kColumnGap, previewTopLeft.y};

        // Difficulty index sanitization is needed before the action pane
        // (DIFFICULTY row reads it) and before the BEST label below.
        if (s.difficultyIdx < 0 ||
            (!cur.difficultyMults.empty() && s.difficultyIdx >= static_cast<int>(cur.difficultyMults.size())))
        {
            s.difficultyIdx = 0;
        }

        // ---- Leaderboard request ----------------------------------------
        // Fire `onRequestLeaderboard` every frame the LevelSelect column
        // is visible. The host de-duplicates by validator and rate-limits
        // wire sends through `LeaderboardCache::shouldRequestScores`, so
        // calling every frame is cheap. Polling each frame is necessary
        // because the client may not be in `LoggedIn_Ready` yet when the
        // user arrives at this screen -- a one-shot fire on selection
        // change would never retry once the handshake completes.
        //
        // We pass the pack-prefixed asset key (same one the preview hook
        // uses), NOT `cur.id` -- the latter is the bare level id and
        // wouldn't pass `HGAssets::isValidLevelId` on the host side.
        if (svc.onRequestLeaderboard && !cur.difficultyMults.empty())
        {
            const sf::base::String& curAssetId = s.filteredLevelIds[s.levelIdx];
            const float             curDM      = cur.difficultyMults[s.difficultyIdx];
            svc.onRequestLeaderboard(curAssetId, curDM);
        }

        if (s.actionIdx < 0 || s.actionIdx >= actionCount)
            s.actionIdx = 0;
        animatedPill(ctx, ctx.cursor, kActionW, s.actionIdx, s.actionSelectionY, s.pane == Pane::Actions);

        // Helper: row index → semantic action.
        const auto activate = [&](ActionRow row)
        {
            switch (row)
            {
                case ActionRow::Favorite:
                {
                    if (profile != nullptr)
                    {
                        const bool   isFav   = profile->isLevelFavorite(cur.id);
                        ProfileData& mutProf = const_cast<ProfileData&>(*profile);
                        if (isFav)
                            mutProf.removeFavoriteLevel(cur.id);
                        else
                            mutProf.addFavoriteLevel(cur.id);
                        if (s.favoritesOnly)
                            s.cacheStale = true;
                    }
                    return false;
                }
                case ActionRow::Play:
                    if (svc.onStartLevel)
                    {
                        const float dm = cur.difficultyMults.empty() ? 1.f : cur.difficultyMults[s.difficultyIdx];
                        svc.onStartLevel(s.filteredLevelIds[s.levelIdx], dm);
                        return true; // caller should `return`
                    }
                    return false;
                case ActionRow::Difficulty:
                    return false; // handled separately via [<]/[>]
                case ActionRow::Back:
                    goBack(app);
                    return true;
            }
            return false;
        };

        // Cycle difficulty in either direction. Used by both keyboard
        // (left/right when this row is focused) and the inline [<]/[>] mouse
        // buttons.
        const auto cycleDiff = [&](int delta)
        {
            if (cur.difficultyMults.empty())
                return;
            const int dN    = static_cast<int>(cur.difficultyMults.size());
            s.difficultyIdx = ((s.difficultyIdx + delta) % dN + dN) % dN;
        };

        // Render rows in the same order indices flow through actionRowAt.
        int row = 0;

        // FAVORITE
        if (hasProfile)
        {
            const bool  isFav = profile->isLevelFavorite(cur.id);
            const char* lbl   = isFav ? "* UNFAVORITE" : "  FAVORITE";
            const bool  foc   = (s.pane == Pane::Actions) && (s.actionIdx == row);
            if (button(ctx, lbl, foc, kActionW))
            {
                s.pane      = Pane::Actions;
                s.actionIdx = row;
                activate(ActionRow::Favorite);
            }
            ++row;
        }

        // PLAY
        {
            const bool foc = (s.pane == Pane::Actions) && (s.actionIdx == row);
            if (button(ctx, "PLAY", foc, kActionW))
            {
                s.pane      = Pane::Actions;
                s.actionIdx = row;
                if (activate(ActionRow::Play))
                    return;
            }
            ++row;
        }

        // DIFFICULTY: row backdrop + label + clickable [<] / [>] buttons.
        // Keyboard: when focused, left/right arrows cycle (the pane-switch
        // logic above already consumed them on this exact frame, so the
        // pane-switch handler must let them through if we're already on
        // Actions -- we simulate that by routing the inline buttons via
        // mouse only and treating the row's focus state as cosmetic).
        // For simplicity, the [<] and [>] mini-buttons handle both mouse
        // and keyboard: when the row is focused, hitting Enter cycles
        // forward (matches Options' "select to confirm" flow).
        {
            const bool foc = (s.pane == Pane::Actions) && (s.actionIdx == row);

            // The label widget paints the row backdrop + truncated text. We
            // then re-position the cursor back onto the same row to overlay
            // the two `[<]`/`[>]` mini-buttons on the right edge. Compact
            // arrows / shortened "DIFF" label keep everything inside the
            // narrower action column that sits next to the preview.
            constexpr float kArrowW   = 28.f;
            constexpr float kArrowGap = 4.f;

            char dbuf[64];
            if (!cur.difficultyMults.empty())
            {
                std::snprintf(dbuf, sizeof(dbuf), "DIFF: %.2f", static_cast<double>(cur.difficultyMults[s.difficultyIdx]));
            }
            else
            {
                std::snprintf(dbuf, sizeof(dbuf), "DIFF: -");
            }

            const float labelLeft = ctx.cursor.x;
            const float labelTop  = ctx.cursor.y;
            label(ctx, dbuf, kActionW - (kArrowW + kArrowGap) * 2.f);

            // Move cursor back to the same row, just to the left of the [<].
            ctx.cursor.y = labelTop;
            ctx.cursor.x = labelLeft + kActionW - (kArrowW + kArrowGap) * 2.f;
            if (button(ctx, "<", false, kArrowW))
            {
                s.pane      = Pane::Actions;
                s.actionIdx = row;
                cycleDiff(-1);
            }
            ctx.cursor.y = labelTop;
            ctx.cursor.x = labelLeft + kActionW - kArrowW;
            if (button(ctx, ">", false, kArrowW))
            {
                s.pane      = Pane::Actions;
                s.actionIdx = row;
                cycleDiff(+1);
            }
            ctx.cursor.x = labelLeft;

            // Keyboard: when this row is focused, Enter cycles forward.
            if (foc && ctx.input.enter)
            {
                cycleDiff(+1);
            }
            ++row;
        }

        // BACK
        {
            const bool foc = (s.pane == Pane::Actions) && (s.actionIdx == row);
            if (button(ctx, "BACK", foc, kActionW))
            {
                s.pane      = Pane::Actions;
                s.actionIdx = row;
                goBack(app);
                return;
            }
            ++row;
        }

        // ---- Metadata pane (full width, beneath preview + actions) -------
        ctx.cursor = ctx.origin = metadataCursor;

        // Local printf-style label that uses the wider details-column width
        // so long author / music / pack names don't overflow the backdrop.
        const auto wlabel = [&](const char* fmtStr, auto... args)
        {
            char buf[256];
            std::snprintf(buf, sizeof(buf), fmtStr, args...);
            label(ctx, buf, kDetailsW);
        };

        wlabel("BY  %.*s", static_cast<int>(cur.author.size()), cur.author.cStr());

        if (!cur.description.empty())
        {
            const auto descLen = std::min<sf::base::SizeT>(cur.description.size(), 280);
            wlabel("%.*s", static_cast<int>(descLen), cur.description.cStr());
        }

        newLine(ctx, 4.f);
        separator(ctx);
        newLine(ctx, 4.f);

        // Best score row (read-only -- difficulty is its own action above).
        if (profile != nullptr && !cur.difficultyMults.empty())
        {
            const float dm   = cur.difficultyMults[s.difficultyIdx];
            const float best = profile->getScore(cur.getValidator(dm));
            if (best > 0.f)
                wlabel("BEST:  %.2fs", static_cast<double>(best));
            else
                label(ctx, "BEST:  N/A", kDetailsW);
        }

        // Helper to print "<key>:  <name>  -  <author>", falling back to
        // "<key>:  <name>" when author is empty (music + pack metadata).
        const auto byline = [&](const char* key, sf::base::StringView name, sf::base::StringView author)
        {
            if (!author.empty())
            {
                wlabel("%s  %.*s  -  %.*s",
                       key,
                       static_cast<int>(name.size()),
                       name.data(),
                       static_cast<int>(author.size()),
                       author.data());
            }
            else
            {
                wlabel("%s  %.*s", key, static_cast<int>(name.size()), name.data());
            }
        };

        if (!cur.musicId.empty())
        {
            const MusicData& md = const_cast<HGAssets&>(assets).getMusicData(cur.packId, cur.musicId);
            byline("MUSIC: ", md.name.toStringView(), md.author.toStringView());
        }

#if 0
        if (!cur.packId.empty())
        {
            const PackData& pd = const_cast<HGAssets&>(assets).getPackData(cur.packId);
            byline("PACK:  ", pd.name.toStringView(), pd.author.toStringView());
        }
#endif

        // ---- Leaderboard column (far right) -----------------------------
        // Sits to the right of the details column, top-aligned with the
        // preview row. Reads `Services::leaderboardScores` /
        // `Services::leaderboardStatus`, populated by the host from
        // `LeaderboardCache` for the (level, difficulty) we requested
        // above.
        constexpr float kLeaderboardW       = 440.f;
        constexpr float kLeaderboardGap     = 20.f;
        const float     leaderboardLeft     = detailsLeft + kDetailsW + kLeaderboardGap;
        const float     headerSize          = ctx.fontSize * 1.3f;
        const float     headerHeight        = headerSize + 8.f;
        const float     leaderboardTop      = previewTopLeft.y;
        const float     rowHeightLB         = ctx.fontSize + 8.f;
        constexpr int   kMaxLeaderboardRows = 12;

        // Header row backdrop + title.
        ctx.target->draw(
            sf::RectangleShapeData{
                .position  = {leaderboardLeft, leaderboardTop},
                .fillColor = ctx.colRow,
                .size      = {kLeaderboardW, headerHeight},
            },
            ctx.renderStates);
        text(ctx, {leaderboardLeft + 12.f, leaderboardTop}, "LEADERBOARD", headerSize);

        // Body backdrop -- one big rectangle behind all rows so empty-state
        // labels land on the same canvas as score rows.
        const float bodyTop    = leaderboardTop + headerHeight + 4.f;
        const float bodyHeight = rowHeightLB * static_cast<float>(kMaxLeaderboardRows) + 8.f;
        ctx.target->draw(
            sf::RectangleShapeData{
                .position  = {leaderboardLeft, bodyTop},
                .fillColor = ctx.colRow,
                .size      = {kLeaderboardW, bodyHeight},
            },
            ctx.renderStates);

        const auto drawCenteredMessage = [&](const char* msg, sf::Color color)
        {
            const sf::Rect2f bounds = measureText(ctx, msg, ctx.fontSize);
            const float      msgX   = leaderboardLeft + (kLeaderboardW - bounds.size.x) * 0.5f;
            const float      msgY   = bodyTop + (bodyHeight - ctx.fontSize) * 0.5f;
            text(ctx, {msgX, msgY}, msg, ctx.fontSize, color);
        };

        // Empty-state message keyed off the host's status enum. The host
        // separates "offline / no path to fetch" from "still handshaking"
        // from "server doesn't track this level" so the user knows
        // whether waiting will help.
        using LBS = Services::LeaderboardStatus;
        if (svc.leaderboardStatus == LBS::Offline)
        {
            drawCenteredMessage("OFFLINE", ctx.colTextDim);
        }
        else if (svc.leaderboardStatus == LBS::Connecting)
        {
            drawCenteredMessage("CONNECTING...", ctx.colTextDim);
        }
        else if (svc.leaderboardStatus == LBS::Unsupported)
        {
            drawCenteredMessage("NOT TRACKED", ctx.colTextDim);
        }
        else if (svc.leaderboardStatus == LBS::Loading || svc.leaderboardScores == nullptr)
        {
            drawCenteredMessage("LOADING...", ctx.colTextDim);
        }
        else if (svc.leaderboardScores->empty())
        {
            drawCenteredMessage("NO SCORES", ctx.colTextDim);
        }
        else
        {
            // Column layout within `kLeaderboardW`:
            //   [12px pad] [#rank 60] [name expands] [time right-aligned] [12px pad]
            constexpr float kRankColW = 60.f;
            constexpr float kPadX     = 12.f;
            const float     nameLeft  = leaderboardLeft + kPadX + kRankColW;
            const float     timeRight = leaderboardLeft + kLeaderboardW - kPadX;
            const float     nameMaxW  = (timeRight - kPadX) - nameLeft;

            const auto& scores = *svc.leaderboardScores;
            const int   nRows  = static_cast<int>(std::min<sf::base::SizeT>(scores.size(), kMaxLeaderboardRows));

            // Defensive: the upstream branches already rule out the
            // empty-scores case, but keep a safety net so the
            // `std::clamp(idx, 0, nRows - 1)` below isn't UB if a future
            // refactor hoists this earlier.
            if (nRows <= 0)
            {
                return;
            }

            // Focus pill: shared `animatedPill` helper, parameterised
            // with our tighter `rowHeightLB` so the height matches
            // leaderboard rows instead of `ctx.rowHeight`.
            const int focusedClamped = std::clamp(s.leaderboardIdx, 0, nRows - 1);
            animatedPill(ctx,
                         {leaderboardLeft, bodyTop + 4.f},
                         kLeaderboardW,
                         focusedClamped,
                         s.leaderboardSelectionY,
                         s.pane == Pane::Leaderboard,
                         rowHeightLB);

            for (int i = 0; i < nRows; ++i)
            {
                const Database::ProcessedScore& ps = scores[static_cast<sf::base::SizeT>(i)];

                const float rowY  = bodyTop + 4.f + rowHeightLB * static_cast<float>(i);
                const float textY = rowY + (rowHeightLB - ctx.fontSize) * 0.5f;

                // Rank (#1, #2, ...) -- accent so the gradient shader picks
                // it up and the leaderboard column reads as "important".
                char rankBuf[16];
                std::snprintf(rankBuf, sizeof(rankBuf), "#%u", static_cast<unsigned>(ps.position));
                text(ctx, {leaderboardLeft + kPadX, textY}, rankBuf, ctx.fontSize, ctx.colAccent);

                // Name -- truncate to fit the available column width.
                char nameBuf[64];
                std::snprintf(nameBuf,
                              sizeof(nameBuf),
                              "%.*s",
                              static_cast<int>(std::min<sf::base::SizeT>(ps.userName.size(), sizeof(nameBuf) - 1)),
                              ps.userName.c_str());
                text(ctx, {nameLeft, textY}, nameBuf, ctx.fontSize, ctx.colText, nameMaxW);

                // Time -- right-aligned at the column edge.
                char timeBuf[32];
                std::snprintf(timeBuf, sizeof(timeBuf), "%.2fs", ps.scoreValue);
                const sf::Rect2f tb = measureText(ctx, timeBuf, ctx.fontSize);
                text(ctx, {timeRight - tb.size.x, textY}, timeBuf, ctx.fontSize, ctx.colText);

                // "(NO REPLAY)" suffix for rows the server confirmed
                // have no stored replay file. Drawn dim and
                // overlapping the time column from the left so it's
                // visible without expanding the row layout.
                const bool noReplay = (svc.leaderboardUnavailable != nullptr) &&
                                      svc.leaderboardUnavailable->contains(ps.scoreTimestamp);
                if (noReplay)
                {
                    constexpr float  kNoReplayCharSize = 0.7f; // tiny relative to row text
                    const float      labelSize         = ctx.fontSize * kNoReplayCharSize;
                    const char*      labelStr          = "(NO REPLAY)";
                    const sf::Rect2f lb                = measureText(ctx, labelStr, labelSize);
                    // Place left of the time column, leaving a small
                    // gap. If the name happens to be long enough to
                    // overlap, the row's truncation kept it within
                    // `nameMaxW` so the label still sits free.
                    const float labelX = timeRight - tb.size.x - lb.size.x - 8.f;
                    const float labelY = rowY + (rowHeightLB - labelSize) * 0.5f;
                    text(ctx, {labelX, labelY}, labelStr, labelSize, ctx.colTextDim);
                }
            }
        }
    }
}

} // namespace hg::ui

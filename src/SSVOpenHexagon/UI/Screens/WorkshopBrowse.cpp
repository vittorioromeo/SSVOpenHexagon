// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/String.hpp"

#include <cstdio>

namespace hg::ui
{

namespace
{

constexpr struct
{
    const char*              label;
    Steam::WorkshopQueryMode mode;
} kModeTabs[] = {
    {"POPULAR", Steam::WorkshopQueryMode::MostPopular},
    {"NEWEST", Steam::WorkshopQueryMode::Newest},
    {"TRENDING", Steam::WorkshopQueryMode::Trending},
    {"ALL", Steam::WorkshopQueryMode::All},
};

constexpr int kModeTabCount = static_cast<int>(sizeof(kModeTabs) / sizeof(*kModeTabs));

void fireQuery(WorkshopBrowseScreenState& s, Services& svc)
{
    if (svc.steamManager == nullptr)
        return;
    svc.steamManager->query_workshop_items(s.queryMode, s.page);
    s.queryInFlight = true;
    std::snprintf(s.statusMessage, sizeof(s.statusMessage), "Querying workshop...");
}

[[nodiscard]] bool itemPasses(const Steam::WorkshopItem& it, sf::base::StringView search, bool downloadedOnly) noexcept
{
    if (downloadedOnly && !it.isInstalled)
        return false;
    if (search.empty())
        return true;
    return containsCI(it.title.toStringView(), search) || containsCI(it.description.toStringView(), search);
}

} // namespace

void drawWorkshopBrowseScreen(Context& ctx, App& app, Services& svc)
{
    WorkshopBrowseScreenState& s = app.workshop;

    if (svc.steamManager == nullptr)
    {
        beginScreen(ctx, screenOrigin(ctx), "WORKSHOP");
        label(ctx, "(Steam is not initialized)");
        if (button(ctx, "BACK", true))
            goBack(app);
        return;
    }

    // First entry: kick off a default query.
    if (!s.initialQueryFired)
    {
        s.initialQueryFired = true;
        fireQuery(s, svc);
    }

    // ---- Input -----------------------------------------------------------
    if (handleEscape(ctx, app))
        return;

    // Build a filtered view of `s.items` according to the search box.
    // Indices are into `s.items`; `selectedIdx` is into this filtered list.
    sf::base::Vector<int> filtered;
    filtered.reserve(s.items.size());
    {
        const auto search = s.search.toStringView();
        for (sf::base::SizeT i = 0; i < s.items.size(); ++i)
        {
            if (itemPasses(s.items[i], search, s.downloadedOnly))
            {
                filtered.emplaceBack(static_cast<int>(i));
            }
        }
    }

    const int n = static_cast<int>(filtered.size());
    if (s.selectedIdx < 0 || s.selectedIdx >= n)
        s.selectedIdx = 0;

    // Three-pane keyboard navigation. Pane indices: 0 = sidebar,
    // 1 = list, 2 = actions. Left/right hop via `paneSwitchLeftRight`;
    // up/down navigate the active pane.
    //
    // Sidebar layout: 4 mode tabs + 3 pagination rows + DOWNLOADED-only
    // toggle. Indices kept stable so input handlers can compare directly.
    constexpr int kSidebarCount   = 8; // 0..3 modes, 4 prev, 5 next, 6 refresh, 7 dl-only
    constexpr int kActionRowCount = 2;
    constexpr int kPaneSidebar    = 0;
    constexpr int kPaneList       = 1;
    constexpr int kPaneActions    = 2;

    // The actions pane is only meaningful when there's an item to act on.
    // Skip past it during pane-switching when the list is empty.
    const int paneCount = (n > 0) ? 3 : 2;
    if (s.activePane >= paneCount)
        s.activePane = paneCount - 1;
    paneSwitchLeftRight(ctx, svc, s.activePane, paneCount);

    navigatePane(ctx, svc, s.sidebarIdx, kSidebarCount, s.activePane == kPaneSidebar);
    navigatePane(ctx, svc, s.selectedIdx, n, s.activePane == kPaneList);
    navigatePane(ctx, svc, s.actionIdx, kActionRowCount, s.activePane == kPaneActions);

    // Windowing index -- the pill animates inside the visible slice.
    // `animatedPill` (called below) drives the per-frame stepToward for us.
    constexpr int kMaxVisible = 12;
    const int     start       = (s.selectedIdx >= kMaxVisible) ? s.selectedIdx - kMaxVisible + 1 : 0;

    // ---- Layout ----------------------------------------------------------
    const sf::Vec2f origin = screenOrigin(ctx);
    const float     left   = origin.x;
    beginScreen(ctx, origin, "WORKSHOP");

    // ---- Search bar + page indicator + status (above the three columns)
    // Page indicator sits to the right of the search bar (mirroring the
    // counter in the level select toolbar) -- keeps it out of the sidebar
    // where it was previously stealing a button slot.
    {
        bool searchSubmitted = false;
        if (textField(ctx, "SEARCH", s.search, true, searchSubmitted, 420.f))
        {
            s.selectedIdx = 0;
        }
        (void)searchSubmitted;

        // "PAGE current/total" -- total = ⌈totalMatching / kSteamPageSize⌉.
        // Steam UGC pages are 50 items by default; we don't override that
        // when querying. Falls back to "PAGE n" when no query has landed
        // (totalMatching = 0).
        constexpr sf::base::U32 kSteamPageSize = 50u;
        char                    pageBuf[48];
        if (s.totalMatching > 0u)
        {
            const auto totalPages = static_cast<int>((s.totalMatching + kSteamPageSize - 1u) / kSteamPageSize);
            std::snprintf(pageBuf, sizeof(pageBuf), "PAGE %d/%d", s.page, totalPages);
        }
        else
        {
            std::snprintf(pageBuf, sizeof(pageBuf), "PAGE %d", s.page);
        }
        ctx.cursor.x = left + 440.f;
        ctx.cursor.y -= ctx.rowHeight;
        label(ctx, pageBuf, 200.f);
        ctx.cursor.x = left;

        if (s.statusMessage[0] != '\0')
        {
            label(ctx, s.statusMessage);
        }

        newLine(ctx, 6.f);
    }

    // Three-column layout: sidebar (filters) | item list | details.
    constexpr float kSidebarW   = 200.f;
    constexpr float kListW      = 420.f;
    const float     listLeft    = left + kSidebarW + 20.f;
    const float     listTop     = ctx.cursor.y;
    const float     detailsLeft = listLeft + kListW + 20.f;

    // ---- Sidebar (mode tabs + pagination + DOWNLOADED toggle) ------------
    {
        ctx.cursor = ctx.origin = {left, listTop};

        // Pill behind the focused sidebar row, animated like the other panes.
        const bool sidebarActive = (s.activePane == kPaneSidebar);
        animatedPill(ctx, {left, listTop}, kSidebarW, s.sidebarIdx, s.sidebarSelectionY, sidebarActive);

        const auto sbFocused = [&](int idx) { return sidebarActive && s.sidebarIdx == idx; };

        // Mode tabs (rows 0..3). The currently-active mode is also rendered
        // as "focused" so users see what's selected even when keyboard
        // focus has moved on.
        for (int i = 0; i < kModeTabCount; ++i)
        {
            const bool active = (s.queryMode == kModeTabs[i].mode);
            const bool focRow = sbFocused(i);
            if (button(ctx, kModeTabs[i].label, focRow || active, kSidebarW) || (focRow && ctx.input.enter))
            {
                s.queryMode = kModeTabs[i].mode;
                s.page      = 1;
                fireQuery(s, svc);
            }
        }

        // Pagination buttons. The page indicator itself lives outside
        // the sidebar (next to the search bar) so it doesn't consume a
        // navigable row slot -- every button index here corresponds 1:1
        // to a visible row.
        const auto sidebarButton = [&](const char* lbl, int idx, auto&& onActivate)
        {
            const bool foc = sbFocused(idx);
            if (button(ctx, lbl, foc, kSidebarW) || (foc && ctx.input.enter))
            {
                onActivate();
            }
        };

        sidebarButton("PREV",
                      4,
                      [&]
        {
            if (s.page > 1)
            {
                s.page -= 1;
                fireQuery(s, svc);
            }
        });
        sidebarButton("NEXT",
                      5,
                      [&]
        {
            s.page += 1;
            fireQuery(s, svc);
        });
        sidebarButton("REFRESH", 6, [&] { fireQuery(s, svc); });

        // DOWNLOADED-only toggle (row 7). Toggling resets the selected
        // item so it stays in-bounds of the new filtered view.
        {
            const bool foc = sbFocused(7);
            bool       dl  = s.downloadedOnly;
            if (toggle(ctx, "DOWNLOADED", dl, foc, kSidebarW))
            {
                s.downloadedOnly = dl;
                s.selectedIdx    = 0;
            }
        }
    }

    // ---- Item list -------------------------------------------------------
    ctx.cursor = ctx.origin = {listLeft, listTop};

    if (n == 0)
    {
        if (s.queryInFlight)
        {
            label(ctx, "(loading...)");
        }
        else if (!s.search.empty() && !s.items.empty())
        {
            label(ctx, "(no items match search)");
        }
        else
        {
            label(ctx, "(no items - try another mode or page)");
        }
    }
    else
    {
        // Item-list pill -- index is windowed (`selectedIdx - start`)
        // because we scroll inside the visible slice, not the full list.
        animatedPill(ctx, ctx.cursor, 420.f, s.selectedIdx - start, s.selectionY, s.activePane == kPaneList);

        const int end = (start + kMaxVisible < n) ? start + kMaxVisible : n;

        for (int i = start; i < end; ++i)
        {
            const Steam::WorkshopItem& item = s.items[filtered[i]];
            char                       rowBuf[180];
            std::snprintf(rowBuf,
                          sizeof(rowBuf),
                          "%c%c %.*s",
                          item.isInstalled ? '*' : ' ',
                          item.isSubscribed ? '+' : ' ',
                          static_cast<int>(item.title.size()),
                          item.title.cStr());
            if (button(ctx, rowBuf, i == s.selectedIdx, 420.f))
            {
                s.selectedIdx = i;
            }
        }

        // Faded peek of the next item below the visible window.
        if (end < n)
        {
            const Steam::WorkshopItem& item = s.items[filtered[end]];
            char                       buf[180];
            std::snprintf(buf,
                          sizeof(buf),
                          "%c%c %.*s",
                          item.isInstalled ? '*' : ' ',
                          item.isSubscribed ? '+' : ' ',
                          static_cast<int>(item.title.size()),
                          item.title.cStr());
            drawFadedPeekRow(ctx, buf, 420.f);
        }
    }

    // ---- Details pane ----------------------------------------------------
    ctx.cursor = ctx.origin = {detailsLeft, listTop};
    if (n > 0 && s.selectedIdx >= 0 && s.selectedIdx < n)
    {
        const Steam::WorkshopItem& item = s.items[filtered[s.selectedIdx]];

        // Title row: row backdrop + larger highlight-colored title text.
        // Routed through `text()` so the menu's case-folding sanitizer
        // upper-cases workshop titles (which arrive as user-typed casing
        // straight from Steam).
        constexpr float kTitleW     = 420.f;
        const float     titleSize   = ctx.fontSize * 1.3f;
        const float     titleHeight = titleSize + 8.f;
        ctx.target->draw(
            sf::RectangleShapeData{
                .position  = ctx.cursor,
                .fillColor = ctx.colRow,
                .size      = {kTitleW, titleHeight},
            },
            ctx.renderStates);
        char titleBuf[160];
        std::snprintf(titleBuf, sizeof(titleBuf), "%.*s", static_cast<int>(item.title.size()), item.title.cStr());
        truncateToFit(titleBuf, kTitleW - 24.f, titleSize);
        text(ctx, ctx.cursor + sf::Vec2f{12.f, 0.f}, titleBuf, titleSize);
        ctx.cursor.y += titleHeight + 6.f;

        // Preview pane. The workshop item carries a `previewUrl` that
        // Steam returns alongside the metadata; we kick off an async
        // HTTP fetch the first time we see each id, and once the bytes
        // come back the host event-pump decodes them into the cached
        // texture under `s.previewTextures[publishedFileId]`. Until the
        // texture lands we show a "(LOADING PREVIEW...)" placeholder.
        constexpr float kPreviewW   = 420.f;
        constexpr float kPreviewH   = kPreviewW * (9.f / 16.f); // 236.25
        constexpr float kFrameInset = 6.f;

        ctx.target->draw(
            sf::RectangleShapeData{
                .position  = ctx.cursor,
                .fillColor = ctx.colRow,
                .size      = {kPreviewW, kPreviewH},
            },
            ctx.renderStates);

        ctx.target->draw(
            sf::RectangleShapeData{
                .position  = {ctx.cursor.x + kFrameInset, ctx.cursor.y + kFrameInset},
                .fillColor = sf::Color{20, 20, 20, 255},
                .size      = {kPreviewW - kFrameInset * 2.f, kPreviewH - kFrameInset * 2.f},
            },
            ctx.renderStates);

        // First time we see this item, kick off the download. Inserting
        // the empty optional marks "in flight" so we don't re-request.
        if (!item.previewUrl.empty() && svc.steamManager != nullptr &&
            s.previewTextures.find(item.publishedFileId) == s.previewTextures.end())
        {
            s.previewTextures[item.publishedFileId] = sf::base::nullOpt;
            svc.steamManager->request_workshop_preview(item.publishedFileId, item.previewUrl);
        }

        const auto cacheIt = s.previewTextures.find(item.publishedFileId);
        const bool hasTex  = (cacheIt != s.previewTextures.end()) && cacheIt->second.hasValue();

        if (hasTex)
        {
            const sf::Texture& tex     = *cacheIt->second;
            const sf::Vec2u    texSize = tex.getSize();
            if (texSize.x > 0 && texSize.y > 0)
            {
                // Letterbox the image inside the inset frame so its
                // aspect ratio is preserved regardless of source size.
                const float frameW = kPreviewW - kFrameInset * 2.f;
                const float frameH = kPreviewH - kFrameInset * 2.f;
                const float scale  = sf::base::min(frameW / static_cast<float>(texSize.x),
                                                   frameH / static_cast<float>(texSize.y));
                const float drawW  = static_cast<float>(texSize.x) * scale;
                const float drawH  = static_cast<float>(texSize.y) * scale;
                const float drawX  = ctx.cursor.x + kFrameInset + (frameW - drawW) * 0.5f;
                const float drawY  = ctx.cursor.y + kFrameInset + (frameH - drawH) * 0.5f;

                ctx.target->draw(
                    sf::Sprite{
                        .position    = {drawX, drawY},
                        .scale       = {scale, scale},
                        .textureRect = {{0.f, 0.f}, texSize.to<sf::Vec2f>()},
                        .color       = sf::Color{255, 255, 255, static_cast<sf::base::U8>(255.f * ctx.screenAlpha)},
                    },
                    sf::RenderStates{.transform = ctx.renderStates.transform, .view = ctx.renderStates.view, .texture = &tex});
            }
        }
        else
        {
            // Placeholder text -- "(NO PREVIEW)" when there's no URL at
            // all, "(LOADING PREVIEW...)" while the HTTP fetch is in
            // flight or after a decode failure.
            const char*      placeholder = item.previewUrl.empty() ? "(NO PREVIEW)" : "(LOADING PREVIEW...)";
            const sf::Rect2f phBounds    = measureText(ctx, placeholder, ctx.fontSize);
            text(ctx,
                 {ctx.cursor.x + (kPreviewW - phBounds.size.x) * 0.5f - phBounds.position.x,
                  ctx.cursor.y + (kPreviewH - phBounds.size.y) * 0.5f - phBounds.position.y},
                 placeholder,
                 ctx.fontSize);
        }

        ctx.cursor.y += kPreviewH + 8.f;

        labelf(ctx, "ID:  %llu", static_cast<unsigned long long>(item.publishedFileId));
        labelf(ctx, "SIZE:  %.2f MB", static_cast<double>(item.sizeBytes) / (1024.0 * 1024.0));
        labelf(ctx,
               "STATE:  %s%s",
               item.isSubscribed ? "subscribed " : "not subscribed ",
               item.isInstalled ? "/ installed" : "");

        newLine(ctx, 4.f);
        separator(ctx);
        newLine(ctx, 4.f);

        // Description (capped to 380 chars to fit the pane).
        if (!item.description.empty())
        {
            const auto descLen = item.description.size() < 380 ? item.description.size() : 380;
            labelf(ctx, "%.*s", static_cast<int>(descLen), item.description.cStr());
        }

        newLine(ctx, 8.f);

        // Action row order: action[0] = DELETE/DOWNLOAD, action[1] = BACK.
        // `paneSwitchLeftRight` (above) wires right-arrow on the item
        // list to jump in; left-arrow jumps back. `Enter` activates the
        // focused row.
        const int  actionCount     = 2;
        const bool actionsFocused  = (s.activePane == kPaneActions);
        const auto isActionFocused = [&](int idx) { return actionsFocused && s.actionIdx == idx; };

        // Selection pill via the shared helper.
        constexpr float kActionW = 240.f;
        animatedPill(ctx, ctx.cursor, kActionW, s.actionIdx, s.actionSelectionY, actionsFocused);

        // DOWNLOAD / DELETE -- semantics swap based on subscription state,
        // but the row index stays the same (focusable as action 0).
        if (item.isSubscribed)
        {
            const bool foc = isActionFocused(0);
            if (button(ctx, "DELETE", foc, kActionW) || (foc && ctx.input.enter))
            {
                svc.steamManager->unsubscribe_workshop_item(item.publishedFileId);
            }
        }
        else
        {
            const bool foc = isActionFocused(0);
            if (button(ctx, "DOWNLOAD", foc, kActionW) || (foc && ctx.input.enter))
            {
                // Subscribe to declared dependencies first so Steam queues
                // their downloads before the parent. Skip any that are
                // already subscribed (avoids redundant SubscribeItem calls).
                for (sf::base::U64 depId : item.dependencies)
                {
                    if (!svc.steamManager->is_workshop_item_subscribed(depId))
                    {
                        svc.steamManager->subscribe_workshop_item(depId);
                    }
                }
                svc.steamManager->subscribe_workshop_item(item.publishedFileId);
            }
        }

        {
            const bool foc = isActionFocused(1);
            if (button(ctx, "BACK", foc, kActionW) || (foc && ctx.input.enter))
            {
                goBack(app);
                return;
            }
        }
        (void)actionCount;

        // ---- Dependencies pane (right of the details pane) ---------------
        if (!item.dependencies.empty())
        {
            // Linear scan of the host-populated name cache. Counts are
            // small (handful per item) so a map isn't worth it.
            const auto findCachedTitle = [&](sf::base::U64 id) -> const sf::base::String*
            {
                for (const auto& e : s.nameCache)
                {
                    if (e.publishedFileId == id)
                        return &e.title;
                }
                return nullptr;
            };
            const auto alreadyRequested = [&](sf::base::U64 id)
            {
                for (sf::base::U64 r : s.requestedNameLookups)
                {
                    if (r == id)
                        return true;
                }
                return false;
            };

            // Request a one-shot details query for any dep we don't have
            // a name for yet (deduplicated against past requests).
            sf::base::Vector<sf::base::U64> toLookUp;
            for (sf::base::U64 depId : item.dependencies)
            {
                if (findCachedTitle(depId) || alreadyRequested(depId))
                    continue;
                s.requestedNameLookups.emplaceBack(depId);
                toLookUp.emplaceBack(depId);
            }
            if (!toLookUp.empty())
            {
                svc.steamManager->query_workshop_details(toLookUp);
            }

            // Place column to the right of the details column. `detailsLeft`
            // sits at `left + 460.f`; the action buttons are 280.f wide; add
            // some padding.
            const float depsLeft = detailsLeft + 300.f;
            ctx.cursor = ctx.origin = {depsLeft, listTop};

            // The dependency list renders at 0.25x the normal size -- it's
            // background metadata, not something the user reads top-to-
            // bottom. We temporarily shrink `ctx.fontSize` and `rowHeight`
            // (since `label`/`labelf` derive their backdrop from those),
            // and refresh `textCenterOffsetY` so vertical centering stays
            // accurate at the smaller size.
            const float     prevFontSize = ctx.fontSize;
            const float     prevRowH     = ctx.rowHeight;
            const float     prevCenterY  = ctx.textCenterOffsetY;
            constexpr float kSmallScale  = 0.25f;
            ctx.fontSize                 = prevFontSize * kSmallScale;
            ctx.rowHeight                = prevRowH * kSmallScale;
            recomputeTextMetrics(ctx);

            labelf(ctx, "REQUIRES (%d):", static_cast<int>(item.dependencies.size()));

            for (sf::base::U64 depId : item.dependencies)
            {
                const char*             mark  = svc.steamManager->is_workshop_item_subscribed(depId) ? "[v]" : "[ ]";
                const sf::base::String* title = findCachedTitle(depId);
                if (title)
                {
                    labelf(ctx, "  %s %.*s", mark, static_cast<int>(title->size()), title->cStr());
                }
                else
                {
                    labelf(ctx, "  %s (loading...) %llu", mark, static_cast<unsigned long long>(depId));
                }
            }

            // Restore the surrounding font / row metrics.
            ctx.fontSize          = prevFontSize;
            ctx.rowHeight         = prevRowH;
            ctx.textCenterOffsetY = prevCenterY;
        }
    }
    else
    {
        if (button(ctx, "BACK", true, 240.f))
        {
            goBack(app);
            return;
        }
    }
}

} // namespace hg::ui

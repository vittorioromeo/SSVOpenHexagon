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

#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/String.hpp"

#include <cstdio>

namespace hg::ui
{

namespace
{

// We always prefetch in "Newest" order. The catalog is small enough
// (OH's workshop fits in low hundreds) that local sort/search/filter
// covers every UX need the old mode-tabs served, without the
// "search/filter only see the current page" footgun the page-by-page
// model produced.
constexpr Steam::WorkshopQueryMode kPrefetchMode = Steam::WorkshopQueryMode::Newest;

// Window into `filteredIndices` shown at once. PgUp/PgDn shift by one
// of these; PREV/NEXT in the sidebar do the same.
constexpr int kPageSize = 12;

// Kick off a query for the next page of the prefetch drain. Sets
// `queryInFlight`; the screen's per-frame driver won't fire again
// until the `QueryComplete` event clears it.
void fireNextPrefetchPage(WorkshopBrowseScreenState& s, Services& svc)
{
    if (svc.steamManager == nullptr || s.queryInFlight || s.prefetchDone)
        return;

    svc.steamManager->query_workshop_items(kPrefetchMode, s.nextPageToFetch);
    s.queryInFlight = true;
    s.nextPageToFetch += 1;

    if (s.expectedTotal == 0)
        std::snprintf(s.statusMessage, sizeof(s.statusMessage), "Loading workshop...");
}

// Reset prefetch state and start over (REFRESH button, etc.).
void resetCatalog(WorkshopBrowseScreenState& s)
{
    s.catalog.clear();
    s.prefetchedFileIds.clear();
    s.expectedTotal     = 0;
    s.nextPageToFetch   = 1;
    s.prefetchDone      = false;
    // `queryInFlight` is cleared by the QueryComplete handler -- don't
    // touch it here, an outstanding query is still going to land.
    s.statusMessage[0] = '\0';
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

    // Prefetch driver: drain pages one-at-a-time into `s.catalog` until
    // we've got every workshop item. Steam responds via `QueryComplete`
    // which appends results, clears `queryInFlight`, and flips
    // `prefetchDone` when the catalog is full. Each frame we top off
    // the pipeline by firing the next page if not already in flight.
    if (s.catalog.empty() && !s.queryInFlight && !s.prefetchDone)
    {
        fireNextPrefetchPage(s, svc);
    }
    else if (!s.prefetchDone && !s.queryInFlight)
    {
        fireNextPrefetchPage(s, svc);
    }

    // ---- Input -----------------------------------------------------------
    if (handleEscape(ctx, app))
        return;

    // Build a filtered view of `s.catalog` according to search +
    // downloaded-only. `filteredIndices` is rebuilt from scratch each
    // frame -- O(N) over the catalog -- which is fine for OH's workshop
    // size and dodges every cache-invalidation footgun we'd hit
    // otherwise.
    sf::base::Vector<int> filteredIndices;
    filteredIndices.reserve(s.catalog.size());
    {
        const auto search = s.search.toStringView();
        for (sf::base::SizeT i = 0; i < s.catalog.size(); ++i)
        {
            if (itemPasses(s.catalog[i], search, s.downloadedOnly))
            {
                filteredIndices.emplaceBack(static_cast<int>(i));
            }
        }
    }

    const int n = static_cast<int>(filteredIndices.size());
    if (s.selectedIdx < 0 || s.selectedIdx >= n)
        s.selectedIdx = 0;

    // Three-pane keyboard navigation. Pane indices: 0 = sidebar,
    // 1 = list, 2 = actions.
    //
    // Sidebar layout (post-prefetch redesign): PREV, NEXT, REFRESH,
    // DOWNLOADED. The mode tabs are gone -- with the full catalog
    // cached locally there's nothing for them to switch between.
    constexpr int kSidebarPrev      = 0;
    constexpr int kSidebarNext      = 1;
    constexpr int kSidebarRefresh   = 2;
    constexpr int kSidebarDlOnly    = 3;
    constexpr int kSidebarCount     = 4;
    constexpr int kActionRowCount   = 2;
    constexpr int kPaneSidebar      = 0;
    constexpr int kPaneList         = 1;
    constexpr int kPaneActions      = 2;

    // The actions pane is only meaningful when there's an item to act on.
    // Skip past it during pane-switching when the list is empty.
    const int paneCount = (n > 0) ? 3 : 2;
    if (s.activePane >= paneCount)
        s.activePane = paneCount - 1;
    paneSwitchLeftRight(ctx, svc, s.activePane, paneCount);

    // Local pagination: a "page" is just `kPageSize` filtered entries.
    // The current page index is derived from `selectedIdx` so that
    // arrowing through items naturally scrolls the visible window, and
    // jumping to "page 2" via PgDn / NEXT just hops `selectedIdx` over
    // one window. The math falls out: `pageStart = currentPage*kPageSize`.
    const int totalPages = (n + kPageSize - 1) / kPageSize;
    const int currentPage = (n == 0) ? 0 : (s.selectedIdx / kPageSize);

    if (s.activePane == kPaneList && n > 0)
    {
        if (ctx.input.pageDown)
        {
            // Land on the first item of the next page, clamped to n-1.
            const int target = (currentPage + 1) * kPageSize;
            s.selectedIdx = (target < n) ? target : n - 1;
            ctx.input.pageDown = false;
        }
        else if (ctx.input.pageUp && currentPage > 0)
        {
            s.selectedIdx = (currentPage - 1) * kPageSize;
            ctx.input.pageUp = false;
        }
    }

    navigatePane(ctx, svc, s.sidebarIdx, kSidebarCount, s.activePane == kPaneSidebar);
    navigatePane(ctx, svc, s.selectedIdx, n, s.activePane == kPaneList);
    navigatePane(ctx, svc, s.actionIdx, kActionRowCount, s.activePane == kPaneActions);

    // Recompute after `navigatePane` may have moved `selectedIdx`.
    const int pageStart = (n == 0) ? 0 : (s.selectedIdx / kPageSize) * kPageSize;

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

        // "PAGE current/total" against the filtered list. With the
        // prefetch model this is always correct -- `totalPages` reflects
        // the user's actual filtered view, not "however many pages Steam
        // happens to think exist", and we can never overshoot it.
        char pageBuf[48];
        if (totalPages > 0)
            std::snprintf(pageBuf, sizeof(pageBuf), "PAGE %d/%d", currentPage + 1, totalPages);
        else
            std::snprintf(pageBuf, sizeof(pageBuf), "PAGE -/-");

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

    // ---- Sidebar (page nav + DOWNLOADED toggle) --------------------------
    {
        ctx.cursor = ctx.origin = {left, listTop};

        // Pill behind the focused sidebar row, animated like the other panes.
        const bool sidebarActive = (s.activePane == kPaneSidebar);
        animatedPill(ctx, {left, listTop}, kSidebarW, s.sidebarIdx, s.sidebarSelectionY, sidebarActive);

        const auto sbFocused = [&](int idx) { return sidebarActive && s.sidebarIdx == idx; };

        const auto sidebarButton = [&](const char* lbl, int idx, auto&& onActivate)
        {
            const bool foc = sbFocused(idx);
            if (button(ctx, lbl, foc, kSidebarW) || (foc && ctx.input.enter))
                onActivate();
        };

        // PREV / NEXT just shift `selectedIdx` by one page-window worth.
        // No Steam round-trip; everything's already in memory.
        sidebarButton("PREV",
                      kSidebarPrev,
                      [&]
        {
            if (n > 0 && currentPage > 0)
                s.selectedIdx = (currentPage - 1) * kPageSize;
        });
        sidebarButton("NEXT",
                      kSidebarNext,
                      [&]
        {
            if (n > 0 && currentPage + 1 < totalPages)
            {
                const int target = (currentPage + 1) * kPageSize;
                s.selectedIdx    = (target < n) ? target : n - 1;
            }
        });

        // REFRESH wipes the cached catalog and restarts the prefetch.
        // Useful when the user has subscribed to / unsubscribed from items
        // through the Steam overlay and wants the workshop view to catch
        // up to the current Steam state.
        sidebarButton("REFRESH",
                      kSidebarRefresh,
                      [&]
        {
            resetCatalog(s);
            s.selectedIdx = 0;
        });

        // DOWNLOADED-only toggle. Toggling resets the selected item so it
        // stays in-bounds of the new filtered view.
        {
            const bool foc = sbFocused(kSidebarDlOnly);
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
        // Empty-state messages reflect the actual reason. Prefetch in
        // flight is signalled by `!prefetchDone`; once that's true the
        // catalog is the whole workshop and "no items" means the filter
        // is too tight (or nothing's been published yet).
        if (!s.prefetchDone)
            label(ctx, "(loading...)");
        else if (!s.search.empty())
            label(ctx, "(no items match search)");
        else if (s.downloadedOnly)
            label(ctx, "(no installed items)");
        else
            label(ctx, "(no workshop items)");
    }
    else
    {
        // Item-list pill -- index is windowed (`selectedIdx - pageStart`)
        // because we scroll inside the current page.
        animatedPill(ctx, ctx.cursor, 420.f, s.selectedIdx - pageStart, s.selectionY, s.activePane == kPaneList);

        const int end = (pageStart + kPageSize < n) ? pageStart + kPageSize : n;

        const auto formatRow = [](char* buf, sf::base::SizeT bufSize, const Steam::WorkshopItem& item)
        {
            std::snprintf(buf,
                          bufSize,
                          "%c%c %.*s",
                          item.isInstalled ? '*' : ' ',
                          item.isSubscribed ? '+' : ' ',
                          static_cast<int>(item.title.size()),
                          item.title.cStr());
        };

        for (int i = pageStart; i < end; ++i)
        {
            const Steam::WorkshopItem& item = s.catalog[filteredIndices[i]];
            char                       rowBuf[180];
            formatRow(rowBuf, sizeof(rowBuf), item);
            if (button(ctx, rowBuf, i == s.selectedIdx, 420.f))
                s.selectedIdx = i;
        }

        // Faded peek of the next item below the visible window.
        if (end < n)
        {
            const Steam::WorkshopItem& item = s.catalog[filteredIndices[end]];
            char                       buf[180];
            formatRow(buf, sizeof(buf), item);
            drawFadedPeekRow(ctx, buf, 420.f);
        }
    }

    // ---- Details pane ----------------------------------------------------
    ctx.cursor = ctx.origin = {detailsLeft, listTop};
    if (n > 0 && s.selectedIdx >= 0 && s.selectedIdx < n)
    {
        const Steam::WorkshopItem& item = s.catalog[filteredIndices[s.selectedIdx]];

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

        // Download progress indicator. The host (`MenuGame::pumpWorkshopEvents`)
        // refreshes `downloadBytes` / `downloadTotalBytes` every frame from
        // `ISteamUGC::GetItemDownloadInfo`. We only surface it when the
        // item is in flight (subscribed, not yet installed) so completed
        // items don't keep showing a 100% bar.
        if (item.isSubscribed && !item.isInstalled)
        {
            constexpr float kBarW     = 420.f;
            constexpr float kBarH     = 12.f;
            constexpr float kBarFrame = 2.f;

            if (item.downloadTotalBytes > 0)
            {
                const double done    = static_cast<double>(item.downloadBytes);
                const double total   = static_cast<double>(item.downloadTotalBytes);
                const float  pct     = static_cast<float>(done / total);
                const float  clamped = pct < 0.f ? 0.f : (pct > 1.f ? 1.f : pct);

                labelf(ctx,
                       "DOWNLOADING:  %.1f%%  (%.2f / %.2f MB)",
                       static_cast<double>(clamped) * 100.0,
                       done / (1024.0 * 1024.0),
                       total / (1024.0 * 1024.0));

                // Bar lives on the line *below* the text. Capture the cursor
                // AFTER `labelf` advanced it; capturing before would overlap.
                const sf::Vec2f barPos = ctx.cursor;

                // Frame (magenta-sentinel) + dark backdrop + magenta fill.
                // Fill width is clamped to the inner area so subpixel
                // overshoot doesn't paint past the frame edge.
                ctx.target->draw(
                    sf::RectangleShapeData{
                        .position  = barPos,
                        .fillColor = ctx.colAccent,
                        .size      = {kBarW, kBarH},
                    },
                    ctx.renderStates);
                ctx.target->draw(
                    sf::RectangleShapeData{
                        .position  = {barPos.x + kBarFrame, barPos.y + kBarFrame},
                        .fillColor = ctx.colRow,
                        .size      = {kBarW - 2.f * kBarFrame, kBarH - 2.f * kBarFrame},
                    },
                    ctx.renderStates);
                const float fillW = (kBarW - 2.f * kBarFrame) * clamped;
                if (fillW > 0.f)
                {
                    ctx.target->draw(
                        sf::RectangleShapeData{
                            .position  = {barPos.x + kBarFrame, barPos.y + kBarFrame},
                            .fillColor = ctx.colAccent,
                            .size      = {fillW, kBarH - 2.f * kBarFrame},
                        },
                        ctx.renderStates);
                }
                ctx.cursor.y += kBarH + 6.f;
            }
            else
            {
                // Steam knows about the subscription but hasn't started
                // the actual transfer yet (queue / hash phase). No bytes
                // to bar, so just say so explicitly.
                labelf(ctx, "DOWNLOADING:  pending");
            }
        }

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

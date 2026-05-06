// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/System/UnicodeString.hpp"

#include "SFML/Base/String.hpp"

#include <cstdio>

namespace hg::ui
{

namespace
{

constexpr struct
{
    const char*               label;
    Steam::WorkshopQueryMode  mode;
} kModeTabs[] = {
    {"POPULAR",  Steam::WorkshopQueryMode::MostPopular},
    {"NEWEST",   Steam::WorkshopQueryMode::Newest},
    {"TRENDING", Steam::WorkshopQueryMode::Trending},
    {"ALL",      Steam::WorkshopQueryMode::All},
};

constexpr int kModeTabCount = static_cast<int>(sizeof(kModeTabs) / sizeof(*kModeTabs));

void fireQuery(WorkshopBrowseScreenState& s, Services& svc)
{
    if (svc.steamManager == nullptr) return;
    svc.steamManager->query_workshop_items(s.queryMode, s.page);
    s.queryInFlight = true;
    std::snprintf(s.statusMessage, sizeof(s.statusMessage), "Querying workshop...");
}

} // namespace

void drawWorkshopBrowseScreen(Context& ctx, App& app, Services& svc)
{
    WorkshopBrowseScreenState& s = app.workshop;

    if (svc.steamManager == nullptr)
    {
        ctx.cursor = ctx.origin = {static_cast<float>(ctx.target->getSize().x) * 0.5f - 240.f,
                                    static_cast<float>(ctx.target->getSize().y) * 0.30f};
        heading(ctx, "WORKSHOP");
        label(ctx, "(Steam is not initialized)");
        if (button(ctx, "BACK", true))
        {
            goBack(app);
            return;
        }
        return;
    }

    // First entry: kick off a default query.
    if (!s.initialQueryFired)
    {
        s.initialQueryFired = true;
        fireQuery(s, svc);
    }

    // ---- Input -----------------------------------------------------------
    if (ctx.input.escape)
    {
        goBack(app);
        return;
    }

    const int n = static_cast<int>(s.items.size());
    if (n > 0)
    {
        if (ctx.input.up)   s.selectedIdx = (s.selectedIdx + n - 1) % n;
        if (ctx.input.down) s.selectedIdx = (s.selectedIdx + 1) % n;
    }

    // ---- Animation step --------------------------------------------------
    stepToward(s.openProgress, 1.f, ctx.dt, 6.f);
    if (n > 0)
    {
        stepToward(s.selectionY, static_cast<float>(s.selectedIdx) * ctx.rowHeight, ctx.dt, 18.f);
    }

    // ---- Layout ----------------------------------------------------------
    const sf::Vec2u sz   = ctx.target->getSize();
    const float     left = static_cast<float>(sz.x) * 0.06f;
    const float     top  = static_cast<float>(sz.y) * 0.10f;

    ctx.cursor = ctx.origin = {left, top};
    heading(ctx, "WORKSHOP");

    // ---- Mode tabs + page + refresh + status ------------------------------
    {
        for (int i = 0; i < kModeTabCount; ++i)
        {
            const bool focused = (s.queryMode == kModeTabs[i].mode);
            if (button(ctx, kModeTabs[i].label, focused, 160.f))
            {
                s.queryMode = kModeTabs[i].mode;
                s.page      = 1;
                fireQuery(s, svc);
            }
            ctx.cursor.y -= ctx.rowHeight;
            ctx.cursor.x = left + 170.f * static_cast<float>(i + 1);
        }
        ctx.cursor.x = left;
        newLine(ctx);

        // Page navigation row.
        char pageBuf[32] = {};
        std::snprintf(pageBuf, sizeof(pageBuf), "PAGE %d", s.page);
        label(ctx, pageBuf);
        ctx.cursor.y -= ctx.rowHeight;
        ctx.cursor.x = left + 200.f;
        if (button(ctx, "PREV", false, 100.f))
        {
            if (s.page > 1) { s.page -= 1; fireQuery(s, svc); }
        }
        ctx.cursor.y -= ctx.rowHeight;
        ctx.cursor.x = left + 310.f;
        if (button(ctx, "NEXT", false, 100.f))
        {
            s.page += 1; fireQuery(s, svc);
        }
        ctx.cursor.y -= ctx.rowHeight;
        ctx.cursor.x = left + 420.f;
        if (button(ctx, "REFRESH", false, 120.f))
        {
            fireQuery(s, svc);
        }
        ctx.cursor.x = left;
        newLine(ctx, 6.f);

        if (s.statusMessage[0] != '\0')
        {
            label(ctx, s.statusMessage);
        }

        newLine(ctx, 6.f);
    }

    const float listLeft    = left;
    const float listTop     = ctx.cursor.y;
    const float detailsLeft = left + 460.f;

    // ---- Item list -------------------------------------------------------
    ctx.cursor = ctx.origin = {listLeft, listTop};

    if (n == 0)
    {
        label(ctx, s.queryInFlight ? "(loading...)" : "(no items — try another mode or page)");
    }
    else
    {
        const sf::Vec2f pillPos {ctx.cursor.x - 8.f, ctx.cursor.y + s.selectionY - 4.f};
        const sf::Vec2f pillSize{420.f + 16.f, ctx.rowHeight + 8.f};
        ctx.target->draw(sf::RectangleShapeData{
                             .position  = pillPos,
                             .fillColor = ctx.colAccent,
                             .size      = pillSize,
                         },
                         ctx.renderStates);

        const int   maxVisible = 12;
        int         start      = 0;
        if (s.selectedIdx >= maxVisible) start = s.selectedIdx - maxVisible + 1;
        const int end = (start + maxVisible < n) ? start + maxVisible : n;

        for (int i = start; i < end; ++i)
        {
            const Steam::WorkshopItem& item = s.items[i];

            char rowBuf[180] = {};
            std::snprintf(rowBuf, sizeof(rowBuf), "%c%c %.*s",
                          item.isInstalled ? '*' : ' ',
                          item.isSubscribed ? '+' : ' ',
                          static_cast<int>(item.title.size()),
                          item.title.cStr());

            if (button(ctx, rowBuf, i == s.selectedIdx, 420.f))
            {
                s.selectedIdx = i;
            }
        }
    }

    // ---- Details pane ----------------------------------------------------
    ctx.cursor = ctx.origin = {detailsLeft, listTop};
    if (n > 0 && s.selectedIdx >= 0 && s.selectedIdx < n)
    {
        const Steam::WorkshopItem& item = s.items[s.selectedIdx];

        ctx.target->draw(*ctx.font,
                         sf::TextData{
                             .position      = ctx.cursor,
                             .string        = sf::UnicodeString{item.title.cStr()},
                             .characterSize = static_cast<unsigned int>(ctx.fontSize * 1.3f),
                             .fillColor     = ctx.colHighlight,
                         },
                         ctx.renderStates);
        ctx.cursor.y += ctx.rowHeight + 6.f;

        char idBuf[64] = {};
        std::snprintf(idBuf, sizeof(idBuf), "ID:  %llu", static_cast<unsigned long long>(item.publishedFileId));
        label(ctx, idBuf);

        char sizeBuf[64] = {};
        std::snprintf(sizeBuf, sizeof(sizeBuf), "SIZE:  %.2f MB",
                      static_cast<double>(item.sizeBytes) / (1024.0 * 1024.0));
        label(ctx, sizeBuf);

        char stateBuf[64] = {};
        std::snprintf(stateBuf, sizeof(stateBuf), "STATE:  %s%s",
                      item.isSubscribed ? "subscribed " : "not subscribed ",
                      item.isInstalled ? "/ installed" : "");
        label(ctx, stateBuf);

        newLine(ctx, 4.f);
        separator(ctx);
        newLine(ctx, 4.f);

        // Description (capped).
        if (!item.description.empty())
        {
            char descBuf[400] = {};
            std::snprintf(descBuf, sizeof(descBuf), "%.*s",
                          static_cast<int>(item.description.size() < 380 ? item.description.size() : 380),
                          item.description.cStr());
            label(ctx, descBuf);
        }

        newLine(ctx, 8.f);

        if (item.isSubscribed)
        {
            if (button(ctx, "UNSUBSCRIBE", false, 240.f))
            {
                svc.steamManager->unsubscribe_workshop_item(item.publishedFileId);
            }
        }
        else
        {
            if (button(ctx, "SUBSCRIBE & INSTALL", false, 280.f))
            {
                svc.steamManager->subscribe_workshop_item(item.publishedFileId);
            }
        }

        if (button(ctx, "BACK", false, 240.f))
        {
            goBack(app);
            return;
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

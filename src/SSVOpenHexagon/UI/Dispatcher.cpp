// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

namespace hg::ui
{

void pushScreen(App& app, Screen target)
{
    app.backStack.emplaceBack(app.current);
    app.current = target;
}

void goBack(App& app)
{
    if (app.backStack.empty())
    {
        return;
    }
    app.current = app.backStack.back();
    app.backStack.popBack();
}

void drawCurrentScreen(Context& ctx, App& app, Services& svc)
{
    switch (app.current)
    {
        case Screen::Main:           drawMainScreen          (ctx, app, svc); break;
        case Screen::Profile:        drawProfileScreen       (ctx, app, svc); break;
        case Screen::Options:        drawOptionsScreen       (ctx, app, svc); break;
        case Screen::LevelSelect:    drawLevelSelectScreen   (ctx, app, svc); break;
        case Screen::WorkshopBrowse: drawWorkshopBrowseScreen(ctx, app, svc); break;
    }
}

} // namespace hg::ui

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/Screens.hpp"

#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

namespace hg::ui
{

void drawCurrentScreen(Context& ctx, App& app, Services& svc)
{
    switch (app.current)
    {
        case Screen::Main: drawMainScreen(ctx, app, svc); break;
    }
}

} // namespace hg::ui

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/UI/App.hpp"
#include "SSVOpenHexagon/UI/Screens.hpp"
#include "SSVOpenHexagon/UI/Services.hpp"
#include "SSVOpenHexagon/UI/UI.hpp"

namespace hg::ui
{

void drawOnlineScreen(Context& ctx, App& app, Services& svc)
{
    OnlineScreenState& s = app.online;
    if (handleEscape(ctx, app))
        return;

    beginScreen(ctx, screenOrigin(ctx), "ONLINE");

    labelf(ctx, "STATUS:  %s", app.profileSnapshot.onlineStatus);
    label(ctx, app.profileSnapshot.name);
    newLine(ctx, 8.f);

    // Build the list of currently-applicable actions. Items appear in a
    // stable order so the keyboard cursor sees a consistent index even as
    // capability flags toggle when the connection state changes. BACK is
    // last and signaled by a null callback.
    const ProfileSnapshot& snap = app.profileSnapshot;
    struct Item
    {
        const char*                          label;
        sf::base::FixedFunction<void(), 64>* cb;
    };
    Item items[6];
    int  count = 0;
    if (snap.canConnect)
        items[count++] = {"CONNECT", &svc.onOnlineConnect};
    if (snap.canLogIn)
        items[count++] = {"LOGIN", &svc.onOnlineLogin};
    if (snap.canRegister)
        items[count++] = {"REGISTER", &svc.onOnlineRegister};
    if (snap.canLogOut)
        items[count++] = {"LOGOUT", &svc.onOnlineLogout};
    if (snap.canDisconnect)
        items[count++] = {"DISCONNECT", &svc.onOnlineDisconnect};
    items[count++] = {"BACK", nullptr};

    if (s.selectedIdx < 0 || s.selectedIdx >= count)
        s.selectedIdx = 0;
    navigatePane(ctx, svc, s.selectedIdx, count, /*active=*/true);
    animatedPill(ctx, ctx.cursor, 360.f, s.selectedIdx, s.selectionY, /*active=*/true);

    for (int i = 0; i < count; ++i)
    {
        if (button(ctx, items[i].label, i == s.selectedIdx))
        {
            s.selectedIdx = i;
            if (items[i].cb == nullptr)
            {
                goBack(app);
                return;
            }
            if (*items[i].cb)
                (*items[i].cb)();
        }
    }
}

} // namespace hg::ui

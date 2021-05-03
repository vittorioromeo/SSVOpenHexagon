// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Joystick {

enum class Jdir : int
{
    Unknown = -1,

    Left,
    Right,
    Up,
    Down,

    JoystickDirectionsCount
};

enum class Jid : int
{
    Unknown = -1,

    Select = 0,
    Exit,
    Focus,
    Swap,
    ForceRestart,
    Restart,
    Replay,
    Screenshot,
    NextPack,
    PreviousPack,
    AddToFavorites,
    FavoritesMenu,

    JoystickBindsCount
};

void update(const float deadzone);

void ignoreAllPresses(const bool ignore);
void setJoystickBind(const unsigned int button, const int buttonID);

[[nodiscard]] bool pressed(const Jdir jdir);
[[nodiscard]] bool risingEdge(const Jdir jdir);

[[nodiscard]] bool pressed(const Jid jid);
[[nodiscard]] bool risingEdge(const Jid jid);

} // namespace hg::Joystick

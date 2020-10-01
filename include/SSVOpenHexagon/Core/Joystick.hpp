// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Joystick
{

enum Jid
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
    JoystickBindsCount
};

void update();

void ignoreAllPresses(bool ignore);
void setJoystickBind(const unsigned int button, const int buttonID);

[[nodiscard]] bool leftPressed();
[[nodiscard]] bool leftRisingEdge();

[[nodiscard]] bool rightPressed();
[[nodiscard]] bool rightRisingEdge();

[[nodiscard]] bool upPressed();
[[nodiscard]] bool upRisingEdge();

[[nodiscard]] bool downPressed();
[[nodiscard]] bool downRisingEdge();

[[nodiscard]] bool selectPressed();
[[nodiscard]] bool selectRisingEdge();

[[nodiscard]] bool exitPressed();
[[nodiscard]] bool exitRisingEdge();

[[nodiscard]] bool focusPressed();
[[nodiscard]] bool focusRisingEdge();

[[nodiscard]] bool swapPressed();
[[nodiscard]] bool swapRisingEdge();

[[nodiscard]] bool restartPressed();
[[nodiscard]] bool restartRisingEdge();

[[nodiscard]] bool forceRestartPressed();
[[nodiscard]] bool forceRestartRisingEdge();

[[nodiscard]] bool replayPressed();
[[nodiscard]] bool replayRisingEdge();

[[nodiscard]] bool screenshotPressed();
[[nodiscard]] bool screenshotRisingEdge();

[[nodiscard]] bool nextPackPressed();
[[nodiscard]] bool nextPackRisingEdge();

[[nodiscard]] bool previousPackPressed();
[[nodiscard]] bool previousPackRisingEdge();

} // namespace hg::Joystick
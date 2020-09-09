// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Joystick
{

void update();

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

[[nodiscard]] bool startPressed();
[[nodiscard]] bool startRisingEdge();

[[nodiscard]] bool aPressed();
[[nodiscard]] bool aRisingEdge();

[[nodiscard]] bool bPressed();
[[nodiscard]] bool bRisingEdge();

} // namespace hg::Joystick

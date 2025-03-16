// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace sf {
class Event;
class GraphicsContext;
class RenderTarget;
class RenderWindow;
class Window;
} // namespace sf

namespace hg::Imgui {

[[nodiscard]] bool wantCaptureKeyboard();
[[nodiscard]] bool wantCaptureMouse();

} // namespace hg::Imgui

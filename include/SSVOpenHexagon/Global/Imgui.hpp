// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace sf {
class Event;
class RenderTarget;
class RenderWindow;
} // namespace sf

namespace hg::Imgui {

[[nodiscard]] bool initialize(sf::RenderWindow&);
void shutdown();
[[nodiscard]] bool wantCaptureKeyboard();
[[nodiscard]] bool wantCaptureMouse();
void processEvent(const sf::Event&);
void render(sf::RenderTarget& renderTarget);

} // namespace hg::Imgui

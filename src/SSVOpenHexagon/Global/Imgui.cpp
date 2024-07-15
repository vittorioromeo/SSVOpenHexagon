// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Imgui.hpp"

#ifndef SSVOH_ANDROID
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui-SFML.h>
#endif

#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/Window.hpp>

namespace hg::Imgui {

[[nodiscard]] bool initialize(
    [[maybe_unused]] sf::GraphicsContext& graphicsContext,
    [[maybe_unused]] sf::RenderWindow& window)
{
#ifndef SSVOH_ANDROID
    return ImGui::SFML::Init(graphicsContext, window);
#endif
}

void shutdown()
{
#ifndef SSVOH_ANDROID
    ImGui::SFML::Shutdown();
#endif
}

[[nodiscard]] bool wantCaptureKeyboard()
{
#ifndef SSVOH_ANDROID
    return ImGui::GetIO().WantCaptureKeyboard;
#else
    return false;
#endif
}

[[nodiscard]] bool wantCaptureMouse()
{
#ifndef SSVOH_ANDROID
    return ImGui::GetIO().WantCaptureMouse;
#else
    return false;
#endif
}

void processEvent([[maybe_unused]] const sf::Window& window,
    [[maybe_unused]] const sf::Event& event)
{
#ifndef SSVOH_ANDROID
    ImGui::SFML::ProcessEvent(window, event);
#endif
}

void render([[maybe_unused]] sf::RenderTarget& renderTarget)
{
#ifndef SSVOH_ANDROID
    ImGui::SFML::Render(renderTarget);
#endif
}

} // namespace hg::Imgui

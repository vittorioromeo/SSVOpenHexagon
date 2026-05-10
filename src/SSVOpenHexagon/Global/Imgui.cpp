// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Imgui.hpp"

#ifndef SSVOH_ANDROID
    #include "SFML/ImGui/IncludeImGui.hpp"
#endif

namespace hg::Imgui
{

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

} // namespace hg::Imgui

// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Include as system header to suppress dependency warnings.
#pragma GCC system_header

//
//
// ----------------------------------------------------------------------------
// Windows Header
// ----------------------------------------------------------------------------

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#endif

//
//
// ----------------------------------------------------------------------------
// C++ Standard Library
// ----------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

//
//
// ----------------------------------------------------------------------------
// C Standard Library
// ----------------------------------------------------------------------------

#include <cctype>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

//
//
// ----------------------------------------------------------------------------
// Discord
// ----------------------------------------------------------------------------

#ifndef SSVOH_ANDROID
#include "discord/discord.h"
#endif

//
//
// ----------------------------------------------------------------------------
// Boost
// ----------------------------------------------------------------------------

#include <boost/pfr.hpp>

//
//
// ----------------------------------------------------------------------------
// Libsodium
// ----------------------------------------------------------------------------

#include <sodium.h>

//
//
// ----------------------------------------------------------------------------
// ImGui and ImGui-SFML
// ----------------------------------------------------------------------------

#ifndef SSVOH_ANDROID
#include <imgui.h>
#include <imgui-SFML.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

//
//
// ----------------------------------------------------------------------------
// SQLite and SQLiteORM
// ----------------------------------------------------------------------------

#include <sqlite3.h>
#include <sqlite_orm.h>

//
//
// ----------------------------------------------------------------------------
// SFML
// ----------------------------------------------------------------------------

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/Base/Optional.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/VideoModeUtils.hpp>

//
//
// ----------------------------------------------------------------------------
// vrm-pp
// ----------------------------------------------------------------------------

#include <vrm/pp.hpp>

//
//
// ----------------------------------------------------------------------------
// SSVUtils
// ----------------------------------------------------------------------------

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>
#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Rnd.hpp>
#include <SSVUtils/Internal/PCG/PCG.hpp>

//
//
// ----------------------------------------------------------------------------
// SSVStart
// ----------------------------------------------------------------------------

#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/GameSystem/GameWindow.hpp>
#include <SSVStart/Input/Trigger.hpp>
#include <SSVStart/Utils/Input.hpp>
#include <SSVStart/Utils/SFML.hpp>

//
//
// ----------------------------------------------------------------------------
// SSVMenuSystem
// ----------------------------------------------------------------------------

#include <SSVMenuSystem/SSVMenuSystem.hpp>

//
//
// ----------------------------------------------------------------------------
// SSVOpenHexagon
// ----------------------------------------------------------------------------

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"

//
//
// ----------------------------------------------------------------------------
// Explicit instantiation declarations (defined in `Instantiations.cpp`)
// ----------------------------------------------------------------------------

extern template class std::vector<std::string>;

extern template class sf::base::Optional<int>;
extern template class sf::base::Optional<std::size_t>;
extern template class sf::base::Optional<std::string>;

extern template class std::unordered_map<std::string, float>;
extern template class std::unordered_map<float, std::string>;
extern template class std::unordered_map<std::string, std::string>;

extern template class std::unordered_set<std::string>;

extern template class std::function<void()>;
extern template class std::function<bool()>;
extern template class std::function<std::string()>;

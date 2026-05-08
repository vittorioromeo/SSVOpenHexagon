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

#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/Vector.hpp"

#include <array>
#include <atomic>
#include <bitset>
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
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

//
//
// ----------------------------------------------------------------------------
// C Standard Library
// ----------------------------------------------------------------------------

#include <cctype>
#include <cmath>
#include <csignal>
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

#include "SSVOpenHexagon/Global/StringHash.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Glsl.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"

//
//
// ----------------------------------------------------------------------------
// SSVUtils
// ----------------------------------------------------------------------------

#include "SSVOpenHexagon/Utils/Log.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>
#include <SSVUtils/Core/Utils/Rnd.hpp>
#include <pcg/pcg_random.hpp>

//
//
// ----------------------------------------------------------------------------
// Local game/input system
// ----------------------------------------------------------------------------

#include "SSVOpenHexagon/GameSystem/GameSystem.hpp"
#include "SSVOpenHexagon/GameSystem/GameWindow.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVOpenHexagon/Input/Utils.hpp"

//
//
// ----------------------------------------------------------------------------
// SSVOpenHexagon
// ----------------------------------------------------------------------------

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

//
//
// ----------------------------------------------------------------------------
// Explicit instantiation declarations (defined in `Instantiations.cpp`)
// ----------------------------------------------------------------------------

extern template class sf::base::Vector<sf::base::String>;

extern template class sf::base::Optional<int>;
extern template class sf::base::Optional<sf::base::SizeT>;
extern template class sf::base::Optional<sf::base::String>;

extern template class std::unordered_map<sf::base::String, float>;
extern template class std::unordered_map<float, sf::base::String>;
extern template class std::unordered_map<sf::base::String, sf::base::String>;

extern template class std::unordered_set<sf::base::String>;

extern template class std::function<void()>;
extern template class std::function<bool()>;
extern template class std::function<sf::base::String()>;

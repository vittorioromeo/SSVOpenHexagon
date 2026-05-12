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

#include <random>
#include <stdexcept>
#include <tuple>

//
//
// ----------------------------------------------------------------------------
// C Standard Library
// ----------------------------------------------------------------------------

#include <cctype>
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
#include "SSVOpenHexagon/Utils/Log.hpp"

//
//
// ----------------------------------------------------------------------------
// SSVOpenHexagon
// ----------------------------------------------------------------------------

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

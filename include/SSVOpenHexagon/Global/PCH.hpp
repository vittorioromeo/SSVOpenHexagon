// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Include as system header to suppress dependency warnings.
#pragma GCC system_header

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cctype>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <set>
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

#include <boost/pfr.hpp>

#include <sodium.h>

#ifndef SSVOH_ANDROID
#include <imgui.h>
#include <imgui-SFML.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

#include <sqlite3.h>
#include <sqlite_orm.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <vrm/pp.hpp>

#include <SSVUtils/Core/Detection/Detection.hpp>
#include <SSVUtils/Core/Common/Common.hpp>

#include <SSVUtils/Core/Common/Frametime.hpp>
#include <SSVUtils/Core/Common/LikelyUnlikely.hpp>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>
#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/Utils/Containers.hpp>
#include <SSVUtils/Core/Utils/Math.hpp>
#include <SSVUtils/Core/Utils/Rnd.hpp>
#include <SSVUtils/Internal/PCG/PCG.hpp>
#include <SSVUtils/Timeline/Timeline.hpp>

#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/GameSystem/GameWindow.hpp>
#include <SSVStart/Input/Trigger.hpp>
#include <SSVStart/Utils/Input.hpp>
#include <SSVStart/Utils/SFML.hpp>
#include <SSVStart/Utils/Vector2.hpp>
#include <SSVStart/VertexVector/VertexVector.hpp>

#include <SSVMenuSystem/SSVMenuSystem.hpp>

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/Global/UtilsJson.hpp"

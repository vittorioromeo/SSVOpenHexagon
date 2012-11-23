# ---------- CMAKE

cmake_minimum_required(VERSION 2.8)
set(APP_TARGET "OpenHexagon")
project(${APP_TARGET})

if(NOT CMAKE_BUILD_TYPE) # By default the build type should be Debug.
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "The type of build. Options: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ---------- COMPILER

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=gnu++11")
      # -Wall:   Enable all warnings.
      # -pedantic: Warn on non-standard ISO C++ code.
      # -std=gnu++11: Compile C++11 code with some things to make GCC work.
endif()

# ---------- SOURCE FILES

set(source
  Components/CPlayer.cpp
  Components/CPlayer.h
  Components/CWall.cpp
  Components/CWall.h
  Data/EventData.cpp
  Data/EventData.h
  Data/LevelData.cpp
  Data/LevelData.h
  Data/MusicData.cpp
  Data/MusicData.h
  Data/PatternData.h
  Data/ProfileData.cpp
  Data/ProfileData.h
  Data/StyleData.cpp
  Data/StyleData.h
  Global/Assets.cpp
  Global/Assets.h
  Global/Config.cpp
  Global/Config.h
  Global/Factory.cpp
  Global/Factory.h
  HexagonGame.cpp
  HexagonGame.h
  LuaContext.cpp
  LuaContext.h
  Main.cpp
  MenuGame.cpp
  MenuGame.h
  PatternManager.cpp
  PatternManager.h
  SSVEntitySystem/EntitySystem/Component.cpp
  SSVEntitySystem/EntitySystem/Component.h
  SSVEntitySystem/EntitySystem/Entity.cpp
  SSVEntitySystem/EntitySystem/Entity.h
  SSVEntitySystem/EntitySystem/Manager.cpp
  SSVEntitySystem/EntitySystem/Manager.h
  SSVEntitySystem/EntitySystem/PtrRepository.h
  SSVEntitySystem/SSVEntitySystem.h
  SSVEntitySystem/Utils.h
  SSVStart/GameSystem/Game.cpp
  SSVStart/GameSystem/Game.h
  SSVStart/GameSystem/GameWindow.cpp
  SSVStart/GameSystem/GameWindow.h
  SSVStart/SSVStart.h
  SSVStart/Timeline/Command.cpp
  SSVStart/Timeline/Command.h
  SSVStart/Timeline/Do.cpp
  SSVStart/Timeline/Do.h
  SSVStart/Timeline/Goto.cpp
  SSVStart/Timeline/Goto.h
  SSVStart/Timeline/Timeline.cpp
  SSVStart/Timeline/Timeline.h
  SSVStart/Timeline/TimelineUtils.h
  SSVStart/Timeline/Wait.cpp
  SSVStart/Timeline/Wait.h
  SSVStart/Utils.h
  Utils/HSL.cpp
  Utils/HSL.h
  Utils/Utils.cpp
  Utils/Utils.h)

# ---------- APPLICATION

find_package(SFML 2 REQUIRED COMPONENTS audio system graphics window)
find_package(JsonCpp REQUIRED)
find_package(Lua51 REQUIRED)

include_directories(
.
SSVEntitySystem
SSVStart
# Tell the compiler that the include dirs are part of the system, that way the
# warnings from them will be ignored and they won't break the build due to the
# enourmous amount of warning and error options the compilers have on.
SYSTEM
${SFML_INCLUDE_DIR}
${LUA_INCLUDE_DIR}
${JSONCPP_INCLUDE_DIR})

# The WIN32 flag simply removes the command prompt from Windows applications.
# It's ignored on non-Windows systems.
add_executable(${APP_TARGET} WIN32 ${source})
target_link_libraries(${APP_TARGET}
${SFML_LIBRARIES}
${LUA_LIBRARIES}
${JSONCPP_LIBRARIES})

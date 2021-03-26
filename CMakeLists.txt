cmake_minimum_required(VERSION 3.9)

include(CheckIPOSupported)

# -----------------------------------------------------------------------------
# CPM
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
set(CPM_DOWNLOAD_VERSION 0.25.1)

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION} AND CPM_VERSION STREQUAL CPM_DOWNLOAD_VERSION))
  message(STATUS "Downloading CPM.cmake")
  file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})
# -----------------------------------------------------------------------------

set(BUILD_SHARED_LIBS true)
set(SFML_STATIC_LIBRARIES false)
set(USE_LUAJIT true)

CPMAddPackage(
  NAME SFML
  GITHUB_REPOSITORY SFML/SFML
  VERSION 2.5.1
  GIT_TAG 2f11710abc5aa478503a7ff3f9e654bd2078ebab
)

# -----------------------------------------------------------------------------

if (NOT USE_LUAJIT)

  CPMAddPackage(
    NAME lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    VERSION 5.3.5
    DOWNLOAD_ONLY YES
  )

  if (lua_ADDED)
    # lua has no CMake support, so we create our own target

    FILE(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
    add_library(lua STATIC ${lua_sources})

    target_include_directories(lua
      PUBLIC
        $<BUILD_INTERFACE:${lua_SOURCE_DIR}>
    )
  endif()

endif()

# -----------------------------------------------------------------------------

if (USE_LUAJIT)

  CPMAddPackage(
    NAME luajit
    GIT_REPOSITORY https://github.com/WohlSoft/LuaJIT
    GIT_TAG 73e6a59d6add696be9b5ca3cc61ac6ae6710e5bf
  )

endif()

# -----------------------------------------------------------------------------

CPMAddPackage(
  NAME zlib
  GITHUB_REPOSITORY madler/zlib
  VERSION 1.2.11
  GIT_TAG cacf7f1d4e3d44d871b605da3b647f07d718623f
)

# -----------------------------------------------------------------------------

CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  VERSION 1.82
  GIT_TAG 35b1148efb839381b84de9290d9caf0b66ad7d03
  DOWNLOAD_ONLY YES
)

if (imgui_ADDED)
  # imgui has no CMake support, so we create our own target

  FILE(GLOB imgui_sources ${imgui_SOURCE_DIR}/*.cpp)

  set(IMGUI_DIR ${imgui_SOURCE_DIR})
  set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR})
  set(IMGUI_SOURCES ${imgui_sources})
endif()

# -----------------------------------------------------------------------------

CPMAddPackage(
  NAME imgui-sfml
  GITHUB_REPOSITORY eliasdaler/imgui-sfml
  VERSION 2.2
  GIT_TAG a181782e151e40cdec7a0d98dcec5ddd56252d10
)

# -----------------------------------------------------------------------------

# Include `vrm_cmake`.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/"
    "${CMAKE_SOURCE_DIR}/cmake/modules/"
    "${CMAKE_SOURCE_DIR}/../vrm_cmake/cmake/"
    "${CMAKE_SOURCE_DIR}/extlibs/vrm_cmake/cmake/")

include(vrm_cmake)

message(STATUS "setting required C++ standard to 20")
set(CMAKE_CXX_STANDARD "20")
set(CMAKE_CXX_STANDARD_REQUIRED on)

project(SSVOpenHexagon CXX C)
enable_testing()

set(PROJECT_NAME_UPPER "")
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

set("${PROJECT_NAME_UPPER}_SOURCE_DIR" "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND CMAKE_MODULE_PATH
"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

message(STATUS "initialized project ${PROJECT_NAME}")

set(INC_DIR "include" CACHE STRING "")
set(SRC_DIR "src" CACHE STRING "")

include_directories("./")
include_directories("./${INC_DIR}")

file(GLOB_RECURSE MAIN_FILE "${SRC_DIR}/SSVOpenHexagon/*/main.cpp")

file(GLOB_RECURSE SRC_LIST
     "${INC_DIR}/SSVOpenHexagon/*"
     "${SRC_DIR}/SSVOpenHexagon/*"
     "public/discord/*")

list(REMOVE_ITEM SRC_LIST ${MAIN_FILE})

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

vrm_cmake_include_vc_dependency_once(vc_detection)
vrm_cmake_add_common_compiler_flags()

vrm_cmake_find_extlib(vrm_pp)
vrm_cmake_find_extlib(SSVUtils)
vrm_cmake_find_extlib(SSVMenuSystem)
vrm_cmake_find_extlib(SSVStart)

if(WIN32)
    if(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:4194304")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,4194304 -fpermissive")
    endif()
endif()

check_ipo_supported(RESULT result)
if(result)
    if("${vrm_cmake_build_type_lower}" STREQUAL "release")
        vrm_cmake_message("Enabling LTO")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
else()
    vrm_cmake_message("LTO not supported")
endif()

add_library(SSVOpenHexagonLib STATIC ${SRC_LIST})
add_executable(${PROJECT_NAME} ${MAIN_FILE})

set(PUBLIC_INCLUDE_DIRS "public")

set(SFML_LIBRARIES sfml-system sfml-window sfml-graphics sfml-audio sfml-network)

if(WIN32)
set(PUBLIC_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/steam_api64.dll"
"${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/discord_game_sdk.dll")
else()
set(PUBLIC_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libsteam_api.so"
"${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libdiscord_game_sdk.so")
endif()

if(USE_LUAJIT)
  add_definitions(-DSSVOH_USE_LUAJIT)
  set(LUASRC "${luajit_SOURCE_DIR}/src")
  set(LUALIB libluajit)
else()
  set(LUASRC "${lua_SOURCE_DIR}")
  set(LUALIB lua)
endif()

target_include_directories(SSVOpenHexagonLib SYSTEM PUBLIC ${SFML_SOURCE_DIR}/include
                                                    PUBLIC ${PUBLIC_INCLUDE_DIRS}
                                                    PUBLIC ${zlib_SOURCE_DIR}
                                                    PUBLIC ${zlib_BINARY_DIR}
                                                    PUBLIC ${LUASRC}
                                                    PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(SSVOpenHexagonLib ${SFML_LIBRARIES}
                                        ${LUALIB}
                                        zlib
                                        ${PUBLIC_LIBRARIES}
                                        "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/discord_game_sdk.dll"
                                        ImGui-SFML::ImGui-SFML)

target_link_libraries(${PROJECT_NAME} SSVOpenHexagonLib)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${SFML_SOURCE_DIR}/include
                                                  PUBLIC ${PUBLIC_INCLUDE_DIRS}
                                                  PUBLIC ${zlib_SOURCE_DIR}
                                                  PUBLIC ${zlib_BINARY_DIR}
                                                  PUBLIC ${LUASRC}
                                                  PUBLIC ${imgui_SOURCE_DIR})

if(UNIX AND NOT APPLE)
    target_link_libraries(SSVOpenHexagonLib pthread)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)

# -----------------------------------------------------------------------------
# Workshop uploader tool
if(WIN32)
    set(STEAM_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/steam_api64.dll")
else()
    set(STEAM_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libsteam_api.so")
endif()

add_executable(OHWorkshopUploader "${CMAKE_CURRENT_SOURCE_DIR}/src/OHWorkshopUploader/main.cpp")
target_include_directories(OHWorkshopUploader SYSTEM PUBLIC ${PUBLIC_INCLUDE_DIRS})
target_link_libraries(OHWorkshopUploader ${STEAM_LIBRARIES})

if(UNIX AND NOT APPLE)
    target_link_libraries(OHWorkshopUploader pthread)
endif()

install(TARGETS OHWorkshopUploader RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)

# -----------------------------------------------------------------------------
# Tests.
vrm_check_target()
add_subdirectory(test)

cmake_minimum_required(VERSION 3.9)

# TODO: build timestamp
# string(TIMESTAMP CMAKE_TIMESTAMP)
# message(${CMAKE_TIMESTAMP})

#
#
# -----------------------------------------------------------------------------
# Produce Lua Metadata
# -----------------------------------------------------------------------------

if(FALSE)
    set(SSVOH_PRODUCE_LUA_METADATA TRUE)
    add_definitions(-DSSVOH_PRODUCE_LUA_METADATA)
else()
    set(SSVOH_PRODUCE_LUA_METADATA FALSE)
endif()

#
#
# -----------------------------------------------------------------------------
# Android build
# -----------------------------------------------------------------------------

string(TOLOWER "${CMAKE_SYSTEM_NAME}" SSVOH_SYSTEM_NAME_LOWERCASE)

if(SSVOH_SYSTEM_NAME_LOWERCASE STREQUAL "android")
    set(SSVOH_ANDROID TRUE)
    add_definitions(-DSSVOH_ANDROID)
else()
    set(SSVOH_ANDROID FALSE)
endif()

#
#
# -----------------------------------------------------------------------------
# Colored output
# -----------------------------------------------------------------------------

# From: https://medium.com/@alasher/colored-c-compiler-output-with-ninja-clang-gcc-10bfe7f2b949
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)

if(${FORCE_COLORED_OUTPUT})
    if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        OR (CMAKE_CXX_COMPILER MATCHES "g\\+\\+")
        OR ("${CMAKE_CXX_COMPILER}" STREQUAL "gcc")
        OR ("${CMAKE_CXX_COMPILER}" STREQUAL "g++")
    )
       add_compile_options("-fdiagnostics-color=always")
    elseif(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        OR ("${CMAKE_CXX_COMPILER}" STREQUAL "clang")
        OR ("${CMAKE_CXX_COMPILER}" STREQUAL "clang++")
        OR (CMAKE_CXX_COMPILER MATCHES "clang\\+\\+")
    )
       add_compile_options("-fcolor-diagnostics")
    endif()
endif()

#
#
# -----------------------------------------------------------------------------
# CCache
# -----------------------------------------------------------------------------

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found CCACHE")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

#
#
# -----------------------------------------------------------------------------
# CPM
# -----------------------------------------------------------------------------

file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)

include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

#
#
# -----------------------------------------------------------------------------
# CPM: SFML
# -----------------------------------------------------------------------------

set(BUILD_SHARED_LIBS false)
set(SFML_STATIC_LIBRARIES true)
set(SFML_ENABLE_PCH true)
set(SFML_ENABLE_LIFETIME_TRACKING true)

CPMAddPackage(
    NAME SFML
    GITHUB_REPOSITORY vittorioromeo/SFML
    GIT_TAG 7f253a5e4dc96554f448674bd3e8c70ef9fa283c
)

set(SFML_ENABLE_LIFETIME_TRACKING true)

set_target_properties(sfml-system PROPERTIES UNITY_BUILD OFF)
set_target_properties(sfml-window PROPERTIES UNITY_BUILD OFF)
set_target_properties(sfml-graphics PROPERTIES UNITY_BUILD OFF)
set_target_properties(sfml-audio PROPERTIES UNITY_BUILD OFF)
set_target_properties(sfml-network PROPERTIES UNITY_BUILD OFF)

#
#
# -----------------------------------------------------------------------------
# CPM: luajit
# -----------------------------------------------------------------------------

# TODO (P2): revisit this decision about FFI
set(LUAJIT_DISABLE_FFI true)
set(LUAJIT_DISABLE_FFI ON CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME luajit
    GIT_REPOSITORY https://github.com/vittorioromeo/LuaJIT
    GIT_TAG 4dc5ffbf60b16e54274c470fcc022165685c96fe
)

set(LUAJIT_DISABLE_FFI true)
set(LUAJIT_DISABLE_FFI ON CACHE BOOL "" FORCE)

# Remove linking against libm on MinGW
if(WIN32)
    file(READ "${luajit_SOURCE_DIR}/src/CMakeLists.txt" CONTENTS)
    string(REGEX REPLACE "target_link_libraries\\(libluajit \\\${LIBM_LIBRARIES}\\)" "" STRIPPED1 "${CONTENTS}")
    file(WRITE "${luajit_SOURCE_DIR}/src/CMakeLists.txt" "${STRIPPED1}")
endif()

#
#
# -----------------------------------------------------------------------------
# CPM: zlib
# -----------------------------------------------------------------------------

CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY madler/zlib
    GIT_TAG cacf7f1d4e3d44d871b605da3b647f07d718623f
)

# Remove example binaries from CMakeLists
file(READ "${zlib_SOURCE_DIR}/CMakeLists.txt" CONTENTS)
string(REGEX REPLACE "Example binaries.*" "" STRIPPED1 "${CONTENTS}")
file(WRITE "${zlib_SOURCE_DIR}/CMakeLists.txt" "${STRIPPED1}")

#
#
# -----------------------------------------------------------------------------
# CPM: imgui
# -----------------------------------------------------------------------------

if(NOT SSVOH_ANDROID)
    CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG 04316bd223f98e4c40bd82dc5f2a35c4891e4e5f
        DOWNLOAD_ONLY YES
    )

    if (imgui_ADDED)
        # imgui has no CMake support, so we create our own target

        file(GLOB imgui_sources "${imgui_SOURCE_DIR}/*.cpp")
        file(GLOB imgui_sources_cpp "${imgui_SOURCE_DIR}/misc/cpp/*.cpp")

        list(REMOVE_ITEM imgui_sources "${imgui_SOURCE_DIR}/imgui_demo.cpp")

        set(IMGUI_DIR ${imgui_SOURCE_DIR})
        set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR})
        set(IMGUI_SOURCES ${imgui_sources} ${imgui_sources_cpp})
    endif()
endif()

#
#
# -----------------------------------------------------------------------------
# CPM: imgui-sfml
# -----------------------------------------------------------------------------

if(NOT SSVOH_ANDROID)
    CPMAddPackage(
        NAME imgui-sfml
        GITHUB_REPOSITORY vittorioromeo/imgui-sfml
        GIT_TAG 8a7a1409f895b3fbcc7d6c19504668e923a6f8f6
        )

    set_target_properties(ImGui-SFML PROPERTIES UNITY_BUILD ON)
endif()

#
#
# -----------------------------------------------------------------------------
# CPM: libsodium-cmake
# -----------------------------------------------------------------------------

set(SODIUM_DISABLE_TESTS ON)
set(SODIUM_DISABLE_TESTS ON CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME libsodium-cmake
    GITHUB_REPOSITORY vittorioromeo/libsodium-cmake
    GIT_TAG fd76500b60fcaa341b6fad24cda88c3504df770d
    OPTIONS "SODIUM_DISABLE_TESTS ON" "SODIUM_DISABLE_TESTS ON CACHE BOOL \"\" FORCE"
)

set(SODIUM_DISABLE_TESTS ON)
set(SODIUM_DISABLE_TESTS ON CACHE BOOL "" FORCE)

#
#
# -----------------------------------------------------------------------------
# CPM: Boost.PFR
# -----------------------------------------------------------------------------

CPMAddPackage(
    NAME boostpfr
    GITHUB_REPOSITORY boostorg/pfr
    GIT_TAG b0bf18798c7037ca8a91a1cd2ad2e5798d8f6d46
    DOWNLOAD_ONLY YES
)

if (boostpfr_ADDED)
    set(boostpfr_INCLUDE_DIR ${boostpfr_SOURCE_DIR}/include)
endif()

#
#
# -----------------------------------------------------------------------------
# vrm_cmake
# -----------------------------------------------------------------------------

# Include `vrm_cmake`.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/"
    "${CMAKE_SOURCE_DIR}/cmake/modules/"
    "${CMAKE_SOURCE_DIR}/../vrm_cmake/cmake/"
    "${CMAKE_SOURCE_DIR}/extlibs/vrm_cmake/cmake/")

include(vrm_cmake)

#
#
# -----------------------------------------------------------------------------
# C++ Standard
# -----------------------------------------------------------------------------

message(STATUS "setting required C++ standard to 20")
set(CMAKE_CXX_STANDARD "20")
set(CMAKE_CXX_STANDARD_REQUIRED on)

#
#
# -----------------------------------------------------------------------------
# Project setup
# -----------------------------------------------------------------------------

project(SSVOpenHexagon CXX C)
enable_testing()

set(PROJECT_NAME_UPPER "")
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

set("${PROJECT_NAME_UPPER}_SOURCE_DIR" "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

message(STATUS "initialized project ${PROJECT_NAME}")

#
#
# -----------------------------------------------------------------------------
# Global unity builds
# -----------------------------------------------------------------------------

set(CMAKE_UNITY_BUILD OFF)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 4)

#
#
# -----------------------------------------------------------------------------
# Source and includes
# -----------------------------------------------------------------------------

set(INC_DIR "include" CACHE STRING "")
set(SRC_DIR "src" CACHE STRING "")

include_directories("./")
include_directories("./${INC_DIR}")

file(GLOB_RECURSE MAIN_FILE "${SRC_DIR}/SSVOpenHexagon/*/main.cpp")

if(NOT SSVOH_ANDROID)
    file(GLOB_RECURSE SRC_LIST
        "${INC_DIR}/SSVOpenHexagon/*"
        "${SRC_DIR}/SSVOpenHexagon/*"
        "public/discord/*")
else()
    file(GLOB_RECURSE SRC_LIST
        "${INC_DIR}/SSVOpenHexagon/*"
        "${SRC_DIR}/SSVOpenHexagon/*")
endif()

file(GLOB_RECURSE C_SRC_LIST
     "public/sqlite/*.c")

list(REMOVE_ITEM SRC_LIST ${MAIN_FILE})

set_source_files_properties("public/sqlite/sqlite3.c"
                            "public/sqlite/shell.c" PROPERTIES COMPILE_FLAGS "-w")

#
#
# -----------------------------------------------------------------------------
# Compile commands
# -----------------------------------------------------------------------------

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

#
#
# -----------------------------------------------------------------------------
# Flags and extlibs
# -----------------------------------------------------------------------------

string(TOLOWER "${CMAKE_BUILD_TYPE}" vrm_cmake_build_type_lower)
vrm_cmake_include_vc_dependency_once(vc_detection)

# TODO: decide how to handle flags
# vrm_cmake_add_common_compiler_flags()

#
#
# -----------------------------------------------------------------------------
# LTO
# -----------------------------------------------------------------------------

# include(CheckIPOSupported)
# check_ipo_supported(RESULT result)
#
# if(result)
#     if("${vrm_cmake_build_type_lower}" STREQUAL "release")
#         message(STATUS "Enabling LTO")
#         set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#     endif()
# else()
#     message(STATUS "LTO not supported")
# endif()

#
#
# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------

if(WIN32 AND "${vrm_cmake_build_type_lower}" STREQUAL "release")
    set(SSVOH_BUILD_WIN32_CONSOLE TRUE)
else()
    set(SSVOH_BUILD_WIN32_CONSOLE FALSE)
endif()

add_library(SSVOpenHexagonLibC STATIC ${C_SRC_LIST})
add_library(SSVOpenHexagonLib STATIC ${SRC_LIST})

if(SSVOH_BUILD_WIN32_CONSOLE)
    add_executable(SSVOpenHexagon WIN32 ${MAIN_FILE})
    add_executable(SSVOpenHexagon-Console ${MAIN_FILE})
else()
    add_executable(SSVOpenHexagon ${MAIN_FILE})
endif()

#
#
# -----------------------------------------------------------------------------
# PCH
# -----------------------------------------------------------------------------

target_precompile_headers(
    SSVOpenHexagonLib PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SSVOpenHexagon/Global/PCH.hpp"
)

target_precompile_headers(SSVOpenHexagon REUSE_FROM SSVOpenHexagonLib)

if(SSVOH_BUILD_WIN32_CONSOLE)
    target_precompile_headers(SSVOpenHexagon-Console REUSE_FROM SSVOpenHexagonLib)
endif()

#
#
# -----------------------------------------------------------------------------
# Link and include libs
# -----------------------------------------------------------------------------

set(PUBLIC_INCLUDE_DIRS "public" "public/sqlite" "public/sqlite_orm")

set(SFML_LIBRARIES
    sfml-system sfml-window sfml-graphics sfml-audio sfml-network)

if(NOT SSVOH_ANDROID)
    if(WIN32)
        set(PUBLIC_LIBRARIES
            "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/steam_api64.lib"
            "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/discord_game_sdk.dll.lib"
            "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/sdkencryptedappticket64.lib"
            sodium
        )
    else()
        set(PUBLIC_LIBRARIES
            "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libsteam_api.so"
            "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libdiscord_game_sdk.so"
            "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libsdkencryptedappticket.so"
            Xcursor
            sodium
        )
    endif()
else()
    set(PUBLIC_LIBRARIES sodium)
endif()

target_include_directories(
    SSVOpenHexagonLib SYSTEM PUBLIC ${SFML_SOURCE_DIR}/include
                             PUBLIC ${PUBLIC_INCLUDE_DIRS}
                             PUBLIC ${zlib_SOURCE_DIR}
                             PUBLIC ${zlib_BINARY_DIR}
                             PUBLIC ${luajit_SOURCE_DIR}/src
                             PUBLIC ${boostpfr_INCLUDE_DIR}
)

target_link_libraries(
    SSVOpenHexagonLib ${SFML_LIBRARIES}
                      libluajit
                      zlib
                      ${PUBLIC_LIBRARIES}
)

if(NOT SSVOH_ANDROID)
    target_include_directories(SSVOpenHexagonLib SYSTEM PUBLIC ${imgui_SOURCE_DIR})
    target_link_libraries(SSVOpenHexagonLib ImGui-SFML::ImGui-SFML)
endif()

macro(ssvoh_find_extlib_for_target target extlib)
    list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_SOURCE_DIR}/../${extlib}/cmake/modules/"
        "${CMAKE_SOURCE_DIR}/../${extlib}/cmake/"
        "${CMAKE_SOURCE_DIR}/extlibs/${extlib}/cmake/modules/"
        "${CMAKE_SOURCE_DIR}/extlibs/${extlib}/cmake/")

    find_package("${extlib}" REQUIRED)
    string(TOUPPER "${extlib}" ${extlib}_UPPER)

    target_include_directories(${target} PUBLIC "${${${extlib}_UPPER}_INCLUDE_DIR}")
endmacro()

ssvoh_find_extlib_for_target(SSVOpenHexagonLib vrm_pp)
ssvoh_find_extlib_for_target(SSVOpenHexagonLib SSVUtils)
ssvoh_find_extlib_for_target(SSVOpenHexagonLib SSVMenuSystem)
ssvoh_find_extlib_for_target(SSVOpenHexagonLib SSVStart)

target_link_libraries(SSVOpenHexagon SSVOpenHexagonLib SSVOpenHexagonLibC)

if(SSVOH_BUILD_WIN32_CONSOLE)
    target_link_libraries(SSVOpenHexagon "${CMAKE_SOURCE_DIR}/art/icon256.res" sfml-main)
    target_compile_options(SSVOpenHexagon PRIVATE "-Wl,-subsystem,windows")
    target_link_libraries(SSVOpenHexagon-Console SSVOpenHexagonLib SSVOpenHexagonLibC
                                                 "${CMAKE_SOURCE_DIR}/art/icon256console.res")
endif()

set(SSVOH_INCLUDE_DIRECTORIES ${SFML_SOURCE_DIR}/include
                              ${PUBLIC_INCLUDE_DIRS}
                              ${zlib_SOURCE_DIR}
                              ${zlib_BINARY_DIR}
                              ${LUASRC})

if(NOT SSVOH_ANDROID)
    set(SSVOH_INCLUDE_DIRECTORIES "${SSVOH_INCLUDE_DIRECTORIES} ${imgui_SOURCE_DIR}")
endif()

target_include_directories(
    SSVOpenHexagon SYSTEM PUBLIC ${SSVOH_INCLUDE_DIRECTORIES}
)

if(SSVOH_BUILD_WIN32_CONSOLE)
    target_include_directories(
        SSVOpenHexagon-Console SYSTEM PUBLIC  ${SSVOH_INCLUDE_DIRECTORIES}
    )
endif()

if(UNIX AND NOT APPLE AND NOT SSVOH_ANDROID)
    target_link_libraries(SSVOpenHexagonLib pthread)
endif()

install(
    TARGETS SSVOpenHexagon RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/
)

if(SSVOH_BUILD_WIN32_CONSOLE)
    install(
        TARGETS SSVOpenHexagon-Console RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/
    )
endif()


#
#
# -----------------------------------------------------------------------------
# Workshop uploader tool
# -----------------------------------------------------------------------------

if(NOT SSVOH_ANDROID)
    if(WIN32)
        set(STEAM_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/steam_api64.lib")
    else()
        set(STEAM_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/libsteam_api.so")
    endif()

    add_executable(
        OHWorkshopUploader "${CMAKE_CURRENT_SOURCE_DIR}/src/OHWorkshopUploader/main.cpp"
    )

    target_include_directories(
        OHWorkshopUploader SYSTEM PUBLIC ${PUBLIC_INCLUDE_DIRS}
    )

    target_link_libraries(OHWorkshopUploader ${STEAM_LIBRARIES})

    if(UNIX AND NOT APPLE AND NOT SSVOH_ANDROID)
        target_link_libraries(OHWorkshopUploader pthread)
    endif()

    install(
        TARGETS OHWorkshopUploader RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/
    )
endif()

#
#
# -----------------------------------------------------------------------------
# Server control tool
# -----------------------------------------------------------------------------

if(NOT SSVOH_ANDROID)
    add_executable(
        OHServerControl "${CMAKE_CURRENT_SOURCE_DIR}/src/OHServerControl/main.cpp"
    )

    target_include_directories(
        OHServerControl SYSTEM PUBLIC ${SFML_SOURCE_DIR}/include
    )

    target_link_libraries(OHServerControl sfml-system sfml-network)

    if(UNIX AND NOT APPLE AND NOT SSVOH_ANDROID)
        target_link_libraries(OHServerControl pthread)
    endif()

    install(
        TARGETS OHServerControl RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/
    )
endif()

#
#
# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

vrm_check_target()
add_subdirectory(test)

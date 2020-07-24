cmake_minimum_required(VERSION 3.0)

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

CPMAddPackage(
  NAME SFML
  GITHUB_REPOSITORY SFML/SFML
  VERSION 2.5.1
  GIT_TAG 2f11710abc5aa478503a7ff3f9e654bd2078ebab
)

# -----------------------------------------------------------------------------

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

# -----------------------------------------------------------------------------

CPMAddPackage(
  NAME zlib
  GITHUB_REPOSITORY madler/zlib
  VERSION 1.2.11
  GIT_TAG cacf7f1d4e3d44d871b605da3b647f07d718623f
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

target_include_directories(SSVOpenHexagonLib SYSTEM PUBLIC ${SFML_SOURCE_DIR}/include)
target_include_directories(SSVOpenHexagonLib SYSTEM PUBLIC ${PUBLIC_INCLUDE_DIRS})
target_include_directories(SSVOpenHexagonLib SYSTEM PUBLIC ${zlib_SOURCE_DIR})
target_include_directories(SSVOpenHexagonLib SYSTEM PUBLIC ${zlib_BINARY_DIR})

target_link_libraries(SSVOpenHexagonLib ${SFML_LIBRARIES})
target_link_libraries(SSVOpenHexagonLib lua)
target_link_libraries(SSVOpenHexagonLib zlib)
target_link_libraries(SSVOpenHexagonLib ${PUBLIC_LIBRARIES})
target_link_libraries(SSVOpenHexagonLib "${CMAKE_CURRENT_SOURCE_DIR}/_RELEASE/discord_game_sdk.dll")

target_link_libraries(${PROJECT_NAME} SSVOpenHexagonLib)

# TODO: reduce repetition of includes
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${SFML_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PUBLIC_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${zlib_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${zlib_BINARY_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${lua_SOURCE_DIR})

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

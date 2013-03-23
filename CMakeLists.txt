cmake_minimum_required(VERSION 2.8)
project(SSVOpenHexagon)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type." FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" CACHE STRING "" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/;${CMAKE_MODULE_PATH}")

if(SFML_STATIC_LIBS)
    set(SFML_STATIC_LIBS TRUE)
endif()

find_package(SFML 2 COMPONENTS audio graphics window system network)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
else()
    set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
    message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing both \"include\" and \"lib\" directories).")
endif()


find_package(JsonCpp)
if(JSONCPP_FOUND)
    include_directories(${JSONCPP_INCLUDE_DIR})
else()
    set(JSONCPP_ROOT "" CACHE PATH "JsonCPP top-level directory")
    message("\n-> JsonCPP directory not found. Set JSONCPP_ROOT to JsonCPP's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(LUA)
if(LUA_FOUND)
    include_directories(${LUA_INCLUDE_DIR})
else()
    set(LUA_ROOT "" CACHE PATH "LUA top-level directory")
    message("\n-> LUA directory not found. Set LUA_ROOT to LUA's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVUtils)
if(SSVUTILS_FOUND)
    include_directories(${SSVUTILS_INCLUDE_DIR})
else()
    set(SSVUTILS_ROOT "" CACHE PATH "SSVUtils top-level directory")
    message("\n-> SSVUtils directory not found. Set SSVUTILS_ROOT to SSVUtils' top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVMenuSystem)
if(SSVMENUSYSTEM_FOUND)
    include_directories(${SSVMENUSYSTEM_INCLUDE_DIR})
else()
    set(SSVUTILS_ROOT "" CACHE PATH "SSVUtils top-level directory")
    message("\n-> SSVMenuSystem directory not found. Set SSVMENUSYSTEM_ROOT to SSVMenuSystem's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVEntitySystem)
if(SSVENTITYSYSTEM_FOUND)
    include_directories(${SSVENTITYSYSTEM_INCLUDE_DIR})
else()
    set(SSVENTITYSYSTEM_ROOT "" CACHE PATH "SSVEntitySystem top-level directory")
    message("\n-> SSVMenuSystem directory not found. Set SSVENEITYSYSTEM_ROOT to SSVEntitySystem's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVStart)
if(SSVSTART_FOUND)
    include_directories(${SSVSTART_INCLUDE_DIR})
else()
    set(SSVSTART_ROOT "" CACHE PATH "SSVStart top-level directory")
    message("\n-> SSVStart directory not found. Set SSVSTART_ROOT to SSVStart's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVLuaWrapper)
if(SSVLUAWRAPPER_FOUND)
    include_directories(${SSVLUAWRAPPER_INCLUDE_DIR})
else()
    set(SSVLUAWRAPPER_ROOT "" CACHE PATH "SSVLuaWrapper top-level directory")
    message("\n-> SSVLuaWrapper directory not found. Set SSVLUAWRAPPER_ROOT to SSVLuaWrapper's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVUtilsJson)
if(SSVUTILSJSON_FOUND)
    include_directories(${SSVUTILSJSON_INCLUDE_DIR})
else()
    set(SSVUTILSJSON_ROOT "" CACHE PATH "SSVUtilsJson top-level directory")
    message("\n-> SSVUtilsJson directory not found. Set SSVUTILSJSON_ROOT to SSVUtilsJson's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

set(SSVUTILS_STATIC_LIBS "" CACHE BOOL "Check to link SSVUtils statically.")
set(SSVUTILS_INCLUDE_DIR "" CACHE STRING "SSVUtils include directory")
set(SSVUTILS_LIBRARY "" CACHE STRING "SSVUtils library file")

set(SSVUTILSJSON_STATIC_LIBS "" CACHE BOOL "Check to link SSVUtilsJson statically.")
set(SSVUTILSJSON_INCLUDE_DIR "" CACHE STRING "SSVUtilsJson include directory")
set(SSVUTILSJSON_LIBRARY "" CACHE STRING "SSVUtilsJson library file")

set(SSVSTART_STATIC_LIBS "" CACHE BOOL "Check to link SSVStart statically.")
set(SSVSTART_INCLUDE_DIR "" CACHE STRING "SSVStart include directory")
set(SSVSTART_LIBRARY "" CACHE STRING "SSVStart library file")

set(SSVLUAWRAPPER_STATIC_LIBS "" CACHE BOOL "Check to link SSVLuaWrapper statically.")
set(SSVLUAWRAPPER_INCLUDE_DIR "" CACHE STRING "SSVLuaWrapper include directory")
set(SSVLUAWRAPPER_LIBRARY "" CACHE STRING "SSVLuaWrapper library file")

set(SSVENTITYSYSTEM_STATIC_LIBS "" CACHE BOOL "Check to link SSVEntitySystem statically.")
set(SSVENTITYSYSTEM_INCLUDE_DIR "" CACHE STRING "SSVEntitySystem include directory")
set(SSVENTITYSYSTEM_LIBRARY "" CACHE STRING "SSVEntitySystem library file")

set(SSVMENUSYSTEM_STATIC_LIBS "" CACHE BOOL "Check to link SSVMenuSystem statically.")
set(SSVMENUSYSTEM_INCLUDE_DIR "" CACHE STRING "SSVMenuSystem include directory")
set(SSVMENUSYSTEM_LIBRARY "" CACHE STRING "SSVMenuSystem library file")

set(SSVUTILS_STATIC_LIBS "" CACHE BOOL "Check to link SSVUtils statically.")
set(SFML_INCLUDE_DIR "" CACHE STRING "SFML2 include directory")
set(SFML_SYSTEM_LIBRARY "" CACHE STRING "SFML2 System library file")
set(SFML_WINDOW_LIBRARY "" CACHE STRING "SFML2 Window library file")
set(SFML_GRAPHICS_LIBRARY "" CACHE STRING "SFML2 Graphics library file")
set(SFML_NETWORK_LIBRARY "" CACHE STRING "SFML2 Network library file")
set(SFML_AUDIO_LIBRARY "" CACHE STRING "SFML2 Audio library file")

set(SRC_DIR "src")
set(INC_DIR "include")

set(
    SOURCES
    ${INC_DIR}/${PROJECT_NAME}/Compatibility/Compatibility.h
    ${INC_DIR}/${PROJECT_NAME}/Components/CPlayer.h
    ${INC_DIR}/${PROJECT_NAME}/Components/CWall.h
    ${INC_DIR}/${PROJECT_NAME}/Core/HGStatus.h
    ${INC_DIR}/${PROJECT_NAME}/Core/HexagonGame.h
    ${INC_DIR}/${PROJECT_NAME}/Core/MenuGame.h
    ${INC_DIR}/${PROJECT_NAME}/Data/EventData.h
    ${INC_DIR}/${PROJECT_NAME}/Data/LevelData.h
    ${INC_DIR}/${PROJECT_NAME}/Data/MusicData.h
    ${INC_DIR}/${PROJECT_NAME}/Data/PackData.h
    ${INC_DIR}/${PROJECT_NAME}/Data/PatternData.h
    ${INC_DIR}/${PROJECT_NAME}/Data/ProfileData.h
    ${INC_DIR}/${PROJECT_NAME}/Data/StyleData.h
    ${INC_DIR}/${PROJECT_NAME}/Global/Assets.h
    ${INC_DIR}/${PROJECT_NAME}/Global/Config.h
    ${INC_DIR}/${PROJECT_NAME}/Global/Factory.h
    ${INC_DIR}/${PROJECT_NAME}/Online/Definitions.h
    ${INC_DIR}/${PROJECT_NAME}/Online/Online.h
    ${INC_DIR}/${PROJECT_NAME}/Utils/FPSWatcher.h
    ${INC_DIR}/${PROJECT_NAME}/Utils/Utils.h
    ${SRC_DIR}/${PROJECT_NAME}/Compatibility/Compatibility.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Components/CPlayer.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Components/CWall.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/HGGraphics.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/HGProperties.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/HGScripting.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/HGUpdate.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/HexagonGame.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/MenuGame.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Core/main.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Data/EventData.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Data/LevelData.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Data/MusicData.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Data/PackData.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Data/ProfileData.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Data/StyleData.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Global/Assets.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Global/Config.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Global/Factory.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Online/Online.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Utils/FPSWatcher.cpp
    ${SRC_DIR}/${PROJECT_NAME}/Utils/Utils.cpp
)

include_directories(${INC_DIR}/${PROJECT_NAME})
include_directories(${INC_DIR}/${PROJECT_NAME}/Core)
include_directories(${INC_DIR}/${PROJECT_NAME}/Data)
include_directories(${INC_DIR}/${PROJECT_NAME}/Utils)

add_executable(SSVOpenHexagon ${SOURCES})

target_link_libraries(SSVOpenHexagon ${SSVUTILS_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVUTILSJSON_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVSTART_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVLUAWRAPPER_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVENTITYSYSTEM_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVMENUSYSTEM_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${JSONCPP_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SFML_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${LUA_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVMENUSYSTEM_LIBRARIES})
target_link_libraries(SSVOpenHexagon ${SSVUTILS_LIBRARIES})

if(UNIX)
    install(TARGETS SSVOpenHexagon
		RUNTIME DESTINATION /usr/local/games/SSVOpenHexagon/
    )

    install(DIRECTORY _RELEASE/
                DESTINATION /usr/local/games/SSVOpenHexagon/
    )

    install(PROGRAMS _RELEASE/openhexagon
                DESTINATION /usr/local/bin/
                PERMISSIONS WORLD_EXECUTE WORLD_READ
    )
else(WIN32)
    install(TARGETS SSVOpenHexagon
                RUNTIME DESTINATION ./_RELEASE/)
endif()

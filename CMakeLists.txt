cmake_minimum_required(VERSION 2.8)
project(SSVOpenHexagon)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type." FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" CACHE STRING "" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/;${CMAKE_MODULE_PATH}")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVUtils/cmake/Modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVUtilsJson/cmake/Modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVMenuSystem/cmake/Modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVLuaWrapper/cmake/Modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVEntitySystem/cmake/Modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVStart/cmake/Modules/")

if(SFML_STATIC_LIBS)
    set(SFML_STATIC_LIBS TRUE)
endif()

set(SPARSEHASH_INCLUDE_DIR "" CACHE STRING "google::sparsehash include directory")

find_package(JsonCpp REQUIRED)
if(JSONCPP_FOUND)
	include_directories(${JSONCPP_INCLUDE_DIR})
	message("\nJsonCpp found and included\n")
else()
	set(JSONCPP_ROOT "" CACHE PATH "JsonCpp top-level directory")
	message("\n-> JsonCpp directory not found. Set SSVUTILS_ROOT to SSVUtils' top-level path (containing both \"include\" and \"lib\" directories).")
endif()

set(SFML_STATIC_LIBRARIES FALSE CACHE BOOL "Look for static SFML libraries.")

find_package(SFML 2 COMPONENTS audio graphics window system network)
if(SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIR})
	message("\nSFML found and included\n")
else()
	set(SFML_INCLUDE_DIR "" CACHE STRING "SFML2 include directory")
	set(SFML_SYSTEM_LIBRARY "" CACHE STRING "SFML2 System library file")
	set(SFML_WINDOW_LIBRARY "" CACHE STRING "SFML2 Window library file")
	set(SFML_GRAPHICS_LIBRARY "" CACHE STRING "SFML2 Graphics library file")
	set(SFML_NETWORK_LIBRARY "" CACHE STRING "SFML2 Network library file")
	set(SFML_AUDIO_LIBRARY "" CACHE STRING "SFML2 Audio library file")
	message("\n-> SFML directory not found. Set include and libraries manually.")
endif()

find_package(SSVUtils REQUIRED)
if(SSVUTILS_FOUND)
	include_directories(${SSVUTILS_INCLUDE_DIR})
	message("\nSSVUtils found and included\n")
else()
	set(SSVUTILS_ROOT "" CACHE PATH "SSVUtils top-level directory")
	message("\n-> SSVUtils directory not found. Set SSVUTILS_ROOT to SSVUtils' top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVUtilsJson REQUIRED)
if(SSVUTILSJSON_FOUND)
	include_directories(${SSVUTILSJSON_INCLUDE_DIR})
	message("\nSSVUtilsJson found and included\n")
else()
	set(SSVUTILSJSON_ROOT "" CACHE PATH "SSVUtils top-level directory")
	message("\n-> SSVUtilsJson directory not found. Set SSVUTILSJSON_ROOT to SSVUtilsJson' top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVStart REQUIRED)
if(SSVSTART_FOUND)
	include_directories(${SSVSTART_INCLUDE_DIR})
	message("\nSSVStart found and included\n")
else()
	set(SSVSTART_ROOT "" CACHE PATH "SSVStart top-level directory")
	message("\n-> SSVStart directory not found. Set SSVSTART_ROOT to SSVStart's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVEntitySystem REQUIRED)
if(SSVENTITYSYSTEM_FOUND)
	include_directories(${SSVENTITYSYSTEM_INCLUDE_DIR})
	message("\nSSVEntitySystem found and included\n")
else()
	set(SSVENTITYSYSTEM_ROOT "" CACHE PATH "SSVEntitySystem top-level directory")
	message("\n-> SSVEntitySystem directory not found. Set SSVENTITYSYSTEM_ROOT to SSVEntitySystem's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(LUA)
if(LUA_FOUND)
    include_directories(${LUA_INCLUDE_DIR})
else()
    set(LUA_ROOT "" CACHE PATH "LUA top-level directory")
    message("\n-> LUA directory not found. Set LUA_ROOT to LUA's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVMenuSystem)
if(SSVMENUSYSTEM_FOUND)
    include_directories(${SSVMENUSYSTEM_INCLUDE_DIR})
else()
    set(SSVUTILS_ROOT "" CACHE PATH "SSVUtils top-level directory")
    message("\n-> SSVMenuSystem directory not found. Set SSVMENUSYSTEM_ROOT to SSVMenuSystem's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

find_package(SSVLuaWrapper)
if(SSVLUAWRAPPER_FOUND)
    include_directories(${SSVLUAWRAPPER_INCLUDE_DIR})
else()
    set(SSVLUAWRAPPER_ROOT "" CACHE PATH "SSVLuaWrapper top-level directory")
    message("\n-> SSVLuaWrapper directory not found. Set SSVLUAWRAPPER_ROOT to SSVLuaWrapper's top-level path (containing both \"include\" and \"lib\" directories).")
endif()

set(SRC_DIR "src")
set(INC_DIR "include")

file(GLOB SRC_LIST 
	${INC_DIR}/${PROJECT_NAME}/Compatibility/*.h
	${SRC_DIR}/${PROJECT_NAME}/Compatibility/*.cpp
	${INC_DIR}/${PROJECT_NAME}/Components/*.h
	${SRC_DIR}/${PROJECT_NAME}/Components/*.cpp
	${INC_DIR}/${PROJECT_NAME}/Core/*.h
	${SRC_DIR}/${PROJECT_NAME}/Core/*.cpp
	${INC_DIR}/${PROJECT_NAME}/Data/*.h
	${SRC_DIR}/${PROJECT_NAME}/Data/*.cpp
	${INC_DIR}/${PROJECT_NAME}/Global/*.h
	${SRC_DIR}/${PROJECT_NAME}/Global/*.cpp
	${INC_DIR}/${PROJECT_NAME}/Online/*.h
	${SRC_DIR}/${PROJECT_NAME}/Online/*.cpp
	${INC_DIR}/${PROJECT_NAME}/Utils/*.h
	${SRC_DIR}/${PROJECT_NAME}/Utils/*.cpp
)

include_directories(${SPARSEHASH_INCLUDE_DIR})
include_directories(${INC_DIR})
include_directories(${INC_DIR}/${PROJECT_NAME})
include_directories(${INC_DIR}/${PROJECT_NAME}/Compatibility)
include_directories(${INC_DIR}/${PROJECT_NAME}/Components)
include_directories(${INC_DIR}/${PROJECT_NAME}/Core)
include_directories(${INC_DIR}/${PROJECT_NAME}/Data)
include_directories(${INC_DIR}/${PROJECT_NAME}/Global)
include_directories(${INC_DIR}/${PROJECT_NAME}/Online)
include_directories(${INC_DIR}/${PROJECT_NAME}/Utils)

add_executable(SSVOpenHexagon ${SRC_LIST})

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

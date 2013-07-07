cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type.")
project(SSVOpenHexagon)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -W -pedantic" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-fno-omit-frame-pointer -g3 -gdwarf-2 -O0" CACHE STRING "" FORCE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/;${CMAKE_MODULE_PATH}")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVJsonCpp/cmake/modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVUtils/cmake/modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVUtilsJson/cmake/modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVMenuSystem/cmake/modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVLuaWrapper/cmake/modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVEntitySystem/cmake/modules/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/extlibs/SSVStart/cmake/modules/")

set(SFML_STATIC_LIBRARIES FALSE CACHE BOOL "Look for static SFML libraries.")
find_package(SFML 2 COMPONENTS audio graphics window system network)
if(NOT SFML_FOUND)
	set(SFML_INCLUDE_DIR "" CACHE STRING "SFML2 include directory")
	set(SFML_SYSTEM_LIBRARY "" CACHE STRING "SFML2 System library file")
	set(SFML_WINDOW_LIBRARY "" CACHE STRING "SFML2 Window library file")
	set(SFML_GRAPHICS_LIBRARY "" CACHE STRING "SFML2 Graphics library file")
	set(SFML_NETWORK_LIBRARY "" CACHE STRING "SFML2 Network library file")
	set(SFML_AUDIO_LIBRARY "" CACHE STRING "SFML2 Audio library file")
	message("\n-> SFML directory not found. Set include and libraries manually.")
endif()

find_package(Sparsehash REQUIRED)
find_package(SSVJsonCpp REQUIRED)
find_package(SSVUtils REQUIRED)
find_package(SSVUtilsJson REQUIRED)
find_package(SSVStart REQUIRED)
find_package(SSVEntitySystem REQUIRED)
find_package(LUA REQUIRED)
find_package(SSVMenuSystem REQUIRED)
find_package(SSVLuaWrapper REQUIRED)

set(SRC_DIR "src")
set(INC_DIR "include")

file(GLOB SRC_LIST
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

include_directories(${INC_DIR})
include_directories(${SPARSEHASH_INCLUDE_DIR})
include_directories(${SFML_INCLUDE_DIR})
include_directories(${SSVJSONCPP_INCLUDE_DIR})
include_directories(${SSVUTILS_INCLUDE_DIR})
include_directories(${SSVUTILSJSON_INCLUDE_DIR})
include_directories(${SSVSTART_INCLUDE_DIR})
include_directories(${SSVENTITYSYSTEM_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SSVMENUSYSTEM_INCLUDE_DIR})
include_directories(${SSVLUAWRAPPER_INCLUDE_DIR})
add_executable(SSVOpenHexagon ${SRC_LIST})
target_link_libraries(SSVOpenHexagon ${SSVJSONCPP_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SSVUTILS_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SSVSTART_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SSVLUAWRAPPER_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SSVENTITYSYSTEM_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SSVMENUSYSTEM_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SFML_AUDIO_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SFML_GRAPHICS_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SFML_WINDOW_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SFML_SYSTEM_LIBRARY})
target_link_libraries(SSVOpenHexagon ${SFML_NETWORK_LIBRARY})
target_link_libraries(SSVOpenHexagon ${LUA_LIBRARY})

if(UNIX)
	install(TARGETS SSVOpenHexagon RUNTIME DESTINATION /usr/local/games/SSVOpenHexagon/)
	install(TARGETS SSVOpenHexagon RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)
	install(DIRECTORY _RELEASE/ DESTINATION /usr/local/games/SSVOpenHexagon/)
	install(PROGRAMS _RELEASE/SSVOpenHexagon DESTINATION /usr/local/bin/ PERMISSIONS WORLD_EXECUTE WORLD_READ)
	install(PROGRAMS _RELEASE/SSVOpenHexagon DESTINATION /usr/local/games/SSVOpenHexagon/ PERMISSIONS WORLD_EXECUTE WORLD_READ)
else(WIN32)
	install(TARGETS SSVOpenHexagon RUNTIME DESTINATION ./_RELEASE/)
endif()

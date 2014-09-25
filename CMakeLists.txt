cmake_minimum_required(VERSION 2.8)
project(SSVOpenHexagon)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extlibs/SSVCMake/cmake/modules/;${CMAKE_MODULE_PATH}")
include(SSVCMake)

SSVCMake_setDefaults()
SSVCMake_findSFML()
find_package(LUA REQUIRED)
find_package(ZLIB REQUIRED)
SSVCMake_findExtlib(SSVUtils)
SSVCMake_findExtlib(SSVUtilsJson)
SSVCMake_findExtlib(SSVMenuSystem)
SSVCMake_findExtlib(SSVLuaWrapper)
SSVCMake_findExtlib(SSVEntitySystem)
SSVCMake_findExtlib(SSVStart)

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,4194304 -fpermissive")
endif()

include_directories(${LUA_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${SRC_LIST})
SSVCMake_linkSFML()
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARY})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)

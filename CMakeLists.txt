cmake_minimum_required(VERSION 3.0)

# Include SSVCmake.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/../SSVCMake/cmake/modules/"
    "${CMAKE_SOURCE_DIR}/extlibs/SSVCMake/cmake/modules/")

include(SSVCMake)

vrm_cmake_init_project(SSVOpenHexagon)

SSVCMake_setDefaults()
vrm_cmake_add_common_compiler_flags()

SSVCMake_findSFML()
if(MINGW)
    find_package(boost)
endif()
find_package(LUA REQUIRED)
find_package(ZLIB REQUIRED)
SSVCMake_findExtlib(vrm_pp)
SSVCMake_findExtlib(SSVUtils)
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
if(MINGW)
    target_link_libraries(${PROJECT_NAME} boost_system boost_thread)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)

cmake_minimum_required(VERSION 3.0)

# Include `vrm_cmake`.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/../vrm_cmake/cmake/"
    "${CMAKE_SOURCE_DIR}/extlibs/vrm_cmake/cmake/")

include(vrm_cmake)

vrm_cmake_init_project(SSVOpenHexagon)

set(INC_DIR "include" CACHE STRING "")
set(SRC_DIR "src" CACHE STRING "")

include_directories("./")
include_directories("./${INC_DIR}")

file(GLOB_RECURSE SRC_LIST "${INC_DIR}/*" "${SRC_DIR}/*")

vrm_cmake_add_common_compiler_flags()

if(MINGW)
    find_package(boost REQUIRED)
endif()

find_package(SFML 2 COMPONENTS system window graphics audio network REQUIRED)
find_package(LUA REQUIRED)
find_package(ZLIB REQUIRED)
vrm_cmake_find_extlib(vrm_pp)
vrm_cmake_find_extlib(SSVUtils)
vrm_cmake_find_extlib(SSVMenuSystem)
vrm_cmake_find_extlib(SSVLuaWrapper)
vrm_cmake_find_extlib(SSVEntitySystem)
vrm_cmake_find_extlib(SSVStart)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,4194304 -fpermissive")
endif()

include_directories(${SFML_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARY})

if(MINGW)
    add_definitions(-DSSVOPENHEXAGON_MINGW_STD_THREADS)

    # TODO: find_package for mingw std threads
    include_directories("${CMAKE_SOURCE_DIR}/extlibs/mingw-std-threads")
    find_package(Boost 1.36.0 COMPONENTS thread REQUIRED)
    target_link_libraries(${PROJECT_NAME} boost_system boost_thread)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)

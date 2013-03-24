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
    set(SFML_ROOT 				""	CACHE PATH "SFML top-level directory"		)
	set(SFML_INCLUDE_DIR 		""	CACHE STRING "SFML2 include directory"		)
	set(SFML_WINDOW_LIBRARY 	""	CACHE STRING "SFML2 Window library file"	)
	set(SFML_GRAPHICS_LIBRARY 	"" 	CACHE STRING "SFML2 Graphics library file"	)
	set(SFML_NETWORK_LIBRARY 	"" 	CACHE STRING "SFML2 Network library file"	)
	set(SFML_AUDIO_LIBRARY 		""	CACHE STRING "SFML2 Audio library file"		)
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

##############################################################
######### 			BUILD DEPENDENCIES			##############
##############################################################

# DEPENDENCY : SSVUTILS
# --> DEPENDS: STL

if(SSVUTILS_BUILD_SHARED_LIB)
	set(SSVUTILS_LIB_TYPE SHARED)
else()
	set(SSVUTILS_LIB_TYPE STATIC)
endif()

set(SSVUTILS_SRC_DIR "extlibs/SSVUtils/src")
set(SSVUTILS_INC_DIR "extlibs/SSVUtils/include")

set(
	SSVUTILS_SOURCES
	${SSVUTILS_INC_DIR}/SSVUtils/Delegate/Delegate.h
	${SSVUTILS_INC_DIR}/SSVUtils/Encryption/Base64.h
	${SSVUTILS_INC_DIR}/SSVUtils/Encryption/MD5.h
	${SSVUTILS_INC_DIR}/SSVUtils/Encryption/ObfuscatedValue.h
	${SSVUTILS_INC_DIR}/SSVUtils/FileSystem/FileSystem.h
	${SSVUTILS_INC_DIR}/SSVUtils/Log/Log.h
	${SSVUTILS_INC_DIR}/SSVUtils/MemoryManager/MemoryManager.h
	${SSVUTILS_INC_DIR}/SSVUtils/MemoryManager/Traits/Traits.h
	${SSVUTILS_INC_DIR}/SSVUtils/Timeline/Command.h
	${SSVUTILS_INC_DIR}/SSVUtils/Timeline/Do.h
	${SSVUTILS_INC_DIR}/SSVUtils/Timeline/Go.h
	${SSVUTILS_INC_DIR}/SSVUtils/Timeline/Timeline.h
	${SSVUTILS_INC_DIR}/SSVUtils/Timeline/TimelineManager.h
	${SSVUTILS_INC_DIR}/SSVUtils/Timeline/Wait.h
	${SSVUTILS_INC_DIR}/SSVUtils/Utils/Utils.h
	${SSVUTILS_INC_DIR}/SSVUtils/Utils/UtilsContainers.h
	${SSVUTILS_INC_DIR}/SSVUtils/Utils/UtilsMath.h
	${SSVUTILS_INC_DIR}/SSVUtils/Utils/UtilsString.h
	${SSVUTILS_SRC_DIR}/SSVUtils/Encryption/Base64.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Encryption/MD5.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/FileSystem/FileSystem.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Log/Log.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Timeline/Command.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Timeline/Do.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Timeline/Go.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Timeline/Timeline.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Timeline/TimelineManager.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Timeline/Wait.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Utils/Utils.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Utils/UtilsContainers.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Utils/UtilsMath.cpp
	${SSVUTILS_SRC_DIR}/SSVUtils/Utils/UtilsString.cpp
)

include_directories(${SSVUTILS_INC_DIR})
add_library(SSVUtils ${SSVUTILS_LIB_TYPE} ${SSVUTILS_SOURCES})

if(SSVUTILS_BUILD_SHARED_LIB)
	set_target_properties(SSVUtils PROPERTIES DEBUG_POSTFIX -d)
else()
	set_target_properties(SSVUtils PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(SSVUtils PROPERTIES RELEASE_POSTFIX -s)
endif()

if(SSVUTILS_BUILD_SHARED_LIB)
	set(SSVUTILS_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/libSSVUtils.a)
else()
	set(SSVUTILS_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/libSSVUtils-s.a)
endif()

message(${SSVUTILS_LIBRARIES})

#DEPENDENCY : SSVUtilsJson
#--> DEPENDS: SSVUtils jsoncpp

if(SSVUTILSJSON_BUILD_SHARED_LIB)
	set(SSVUTILSJSON_LIB_TYPE SHARED)
else()
	set(SSVUTILSJSON_LIB_TYPE STATIC)
endif()

set(SSVUTILSJSON_SRC_DIR "extlibs/SSVUtilsJson/src")
set(SSVUTILSJSON_INC_DIR "extlibs/SSVUtilsJson/include")

set(
	SSVUTILSJSON_SOURCES
	${SSVUTILSJSON_INC_DIR}/SSVUtilsJson/Utils/UtilsJson.h
	${SSVUTILSJSON_SRC_DIR}/SSVUtilsJson/Utils/UtilsJson.cpp
)

include_directories(${SSVUTILSJSON_INC_DIR})
add_library(SSVUtilsJson ${SSVUTILSJSON_LIB_TYPE} ${SSVUTILSJSON_SOURCES})

target_link_libraries( SSVUtilsJson SSVUtils)
target_link_libraries( SSVUtilsJson ${JSONCPP_LIBRARIES} )

if(SSVUTILSJSON_BUILD_SHARED_LIB)
	set_target_properties(SSVUtilsJson PROPERTIES DEBUG_POSTFIX -d)
else()
	set_target_properties(SSVUtilsJson PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(SSVUtilsJson PROPERTIES RELEASE_POSTFIX -s)
endif()

if(SSVUTILSJSON_BUILD_SHARED_LIB)
	set(SSVUTILSJSON_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVUtilsJson.a)
else()
	set(SSVUTILSJSON_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVUtilsJson-s.a)
endif()

#DEPENDENCY : SSVStart
#--> DEPENDS: SSVUtils SSVUtilsJson SFML

if(SSVSTART_BUILD_SHARED_LIB)
	set(SSVSTART_LIB_TYPE SHARED)
else()
	set(SSVSTART_LIB_TYPE STATIC)
endif()

set(SSVSTART_SRC_DIR "extlibs/SSVStart/src")
set(SSVSTART_INC_DIR "extlibs/SSVStart/include")

set(
	SSVSTART_SOURCES
	${SSVSTART_INC_DIR}/SSVStart/Assets/AssetFolder.h
	${SSVSTART_INC_DIR}/SSVStart/Assets/AssetManager.h
	${SSVSTART_INC_DIR}/SSVStart/Camera/Camera.h
	${SSVSTART_INC_DIR}/SSVStart/GameSystem/GameState.h
	${SSVSTART_INC_DIR}/SSVStart/GameSystem/GameWindow.h
	${SSVSTART_INC_DIR}/SSVStart/GameSystem/Timers.h
	${SSVSTART_INC_DIR}/SSVStart/Input/Combo.h
	${SSVSTART_INC_DIR}/SSVStart/Input/Manager.h
	${SSVSTART_INC_DIR}/SSVStart/Input/Trigger.h
	${SSVSTART_INC_DIR}/SSVStart/Tileset/Tileset.h
	${SSVSTART_INC_DIR}/SSVStart/Utils/ThreadWrapper.h
	${SSVSTART_INC_DIR}/SSVStart/Utils/Utils.h
	${SSVSTART_SRC_DIR}/SSVStart/Assets/AssetFolder.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Assets/AssetManager.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Camera/Camera.cpp
	${SSVSTART_SRC_DIR}/SSVStart/GameSystem/GameState.cpp
	${SSVSTART_SRC_DIR}/SSVStart/GameSystem/GameWindow.cpp
	${SSVSTART_SRC_DIR}/SSVStart/GameSystem/GWProperties.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Input/Combo.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Input/Manager.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Input/Trigger.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Tileset/Tileset.cpp
	${SSVSTART_SRC_DIR}/SSVStart/Utils/Utils.cpp
)

include_directories(${SSVSTART_INC_DIR})
add_library(SSVStart ${SSVSTART_LIB_TYPE} ${SSVSTART_SOURCES})

target_link_libraries( SSVStart SSVUtils )
target_link_libraries( SSVStart ${SFML_LIBRARIES}     )

if(SSVSTART_BUILD_SHARED_LIB)
	set_target_properties(SSVStart PROPERTIES DEBUG_POSTFIX -d)
else()
	set_target_properties(SSVStart PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(SSVStart PROPERTIES RELEASE_POSTFIX -s)
endif()

if(SSVSTART_BUILD_SHARED_LIB)
	set(SSVSTART_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVStart.a)
else()
	set(SSVSTART_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVStart-s.a)
endif()

#DEPENDENCY : SSVLuaWrapper
#--> DEPENDS: LUA51

if(SSVLUAWRAPPER_BUILD_SHARED_LIB)
	set(SSVLUAWRAPPER_LIB_TYPE SHARED)
else()
	set(SSVLUAWRAPPER_LIB_TYPE STATIC)
endif()

set(SSVLUAWRAPPER_SRC_DIR "extlibs/SSVLuaWrapper/src")
set(SSVLUAWRAPPER_INC_DIR "extlibs/SSVLuaWrapper/include")

set(
	SSVLUAWRAPPER_SOURCES
	${SSVLUAWRAPPER_INC_DIR}/SSVLuaWrapper/LuaContext/LuaContext.h
	${SSVLUAWRAPPER_SRC_DIR}/SSVLuaWrapper/LuaContext/LuaContext.cpp
)

include_directories(${SSVLUAWRAPPER_INC_DIR})
add_library(SSVLuaWrapper ${SSVLUAWRAPPER_LIB_TYPE} ${SSVLUAWRAPPER_SOURCES})

target_link_libraries(SSVLuaWrapper ${LUA_LIBRARIES})

if(SSVLUAWRAPPER_BUILD_SHARED_LIB)
	set_target_properties(SSVLuaWrapper PROPERTIES DEBUG_POSTFIX -d)
else()
	set_target_properties(SSVLuaWrapper PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(SSVLuaWrapper PROPERTIES RELEASE_POSTFIX -s)
endif()

if(SSVSTART_BUILD_SHARED_LIB)
	set(SSVSTART_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVLuaWrapper.a)
else()
	set(SSVSTART_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVLuaWrapper-s.a)
endif()

#DEPENDENCY : SSVEntitySystem
#--> DEPENDS: GoogleSparsehash SSVUtils

if(SSVENTITYSYSTEM_BUILD_SHARED_LIB)
	set(SSVENTITYSYSTEM_LIB_TYPE SHARED)
else()
	set(SSVENTITYSYSTEM_LIB_TYPE STATIC)
endif()

set(SSVENTITYSYSTEM_SRC_DIR "extlibs/SSVEntitySystem/src")
set(SSVENTITYSYSTEM_INC_DIR "extlibs/SSVEntitySystem/include")

set(
	SSVENTITYSYSTEM_SOURCES
	${SSVENTITYSYSTEM_INC_DIR}/SSVEntitySystem/Core/Component.h
	${SSVENTITYSYSTEM_INC_DIR}/SSVEntitySystem/Core/Entity.h
	${SSVENTITYSYSTEM_INC_DIR}/SSVEntitySystem/Core/Manager.h
	${SSVENTITYSYSTEM_INC_DIR}/SSVEntitySystem/Utils/Repository.h
	${SSVENTITYSYSTEM_INC_DIR}/SSVEntitySystem/Utils/Utils.h
	${SSVENTITYSYSTEM_SRC_DIR}/SSVEntitySystem/Core/Component.cpp
	${SSVENTITYSYSTEM_SRC_DIR}/SSVEntitySystem/Core/Entity.cpp
	${SSVENTITYSYSTEM_SRC_DIR}/SSVEntitySystem/Core/Manager.cpp
	${SSVENTITYSYSTEM_SRC_DIR}/SSVEntitySystem/Utils/Utils.cpp
)

include_directories(${SSVENTITYSYSTEM_INC_DIR})
add_library(SSVEntitySystem ${SSVENTITYSYSTEM_LIB_TYPE} ${SSVENTITYSYSTEM_SOURCES})

target_link_libraries(SSVEntitySystem ${SPARSEHASH_LIBRARIES})
target_link_libraries(SSVEntitySystem SSVUtils)

if(SSVENTITYSYSTEM_BUILD_SHARED_LIB)
	set_target_properties(SSVEntitySystem PROPERTIES DEBUG_POSTFIX -d)
else()
	set_target_properties(SSVEntitySystem PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(SSVEntitySystem PROPERTIES RELEASE_POSTFIX -s)
endif()

if(SSVENTITYSYSTEM_BUILD_SHARED_LIB)
	set(SSVENTITYSYSTEM_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVEntitySystem.a)
else()
	set(SSVENTITYSYSTEM_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVEntitySystem-s.a)
endif()


#DEPENDENCY : SSVMenuSystem
#--> DEPENDS: GoogleSparsehash SSVUtils

if(SSVMENUSYSTEM_BUILD_SHARED_LIB)
	set(SSVMENUSYSTEM_LIB_TYPE SHARED)
else()
	set(SSVMENUSYSTEM_LIB_TYPE STATIC)
endif()

set(SSVMENUSYSTEM_SRC_DIR "extlibs/SSVMenuSystem/src")
set(SSVMENUSYSTEM_INC_DIR "extlibs/SSVMenuSystem/include")

set(
	SSVMENUSYSTEM_SOURCES
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Menu/Category.h
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Menu/ItemBase.h
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Menu/Menu.h
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Items/Goto.h
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Items/Single.h
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Items/Slider.h
	${SSVMENUSYSTEM_INC_DIR}/SSVMenuSystem/Items/Toggle.h
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Menu/Category.cpp
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Menu/ItemBase.cpp
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Menu/Menu.cpp
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Items/Goto.cpp
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Items/Single.cpp
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Items/Slider.cpp
	${SSVMENUSYSTEM_SRC_DIR}/SSVMenuSystem/Items/Toggle.cpp
)

include_directories(${SSVMENUSYSTEM_INC_DIR})
add_library(SSVMenuSystem ${SSVMENUSYSTEM_LIB_TYPE} ${SSVMENUSYSTEM_SOURCES})

if(SSVMENUSYSTEM_BUILD_SHARED_LIB)
	set_target_properties(SSVMenuSystem PROPERTIES DEBUG_POSTFIX -d)
else()
	set_target_properties(SSVMenuSystem PROPERTIES DEBUG_POSTFIX -s-d)
	set_target_properties(SSVMenuSystem PROPERTIES RELEASE_POSTFIX -s)
endif()

if(SSVMENUSYSTEM_BUILD_SHARED_LIB)
	set(SSVMENUSYSTEM_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVMenuSystem.a)
else()
	set(SSVMENUSYSTEM_LIBRARIES ${CMAKE_BINARY_DIR}/libSSVMenuSystem-s.a)
endif()


##############################################################
#########			BUILD OPENHEXAGON			##############
##############################################################

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
    #${INC_DIR}/${PROJECT_NAME}/Data/PatternData.h
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

include_directories(${INC_DIR})
include_directories(${INC_DIR}/${PROJECT_NAME})
include_directories(${INC_DIR}/${PROJECT_NAME}/Core)
include_directories(${INC_DIR}/${PROJECT_NAME}/Data)
include_directories(${INC_DIR}/${PROJECT_NAME}/Utils)

add_executable(SSVOpenHexagon ${SOURCES})

target_link_libraries(SSVOpenHexagon SSVUtils)
target_link_libraries(SSVOpenHexagon SSVUtilsJson)
target_link_libraries(SSVOpenHexagon SSVStart)
target_link_libraries(SSVOpenHexagon SSVLuaWrapper)
target_link_libraries(SSVOpenHexagon SSVEntitySystem)
target_link_libraries(SSVOpenHexagon SSVMenuSystem)
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
                RUNTIME DESTINATION .
	)
	
	install(DIRECTORY _RELEASE/
				DESTINATION "C:/Program Files (x86)/SSVOpenHexagon/"
	)
endif()

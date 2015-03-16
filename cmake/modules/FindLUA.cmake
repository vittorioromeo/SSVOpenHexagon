FIND_PATH(LUA_INCLUDE_DIR lua.hpp
  PATH_SUFFIXES include/ lua/ lua/include/ ./ lua5.2/ lua5.2/include/ src/ lua/src/ lua5.2/src/
  PATHS "${PROJECT_SOURCE_DIR}/../lua/"
  "${PROJECT_SOURCE_DIR}/extlibs/lua/"
  ${LUA_ROOT}
  $ENV{LUA_ROOT}
  /usr/local/
  /usr/
  /sw/
  /opt/local/
  /opt/csw/
  /opt/
)

message("\nFound Lua include at: ${LUA_INCLUDE_DIR}.\n")

FIND_LIBRARY(LUA_LIBRARY
  NAMES liblua5.2 lua5.2 lua liblua
  PATH_SUFFIXES lib/ lib/x86_64-linux-gnu/ lib64/ lua/ lua/lib/ lua/lib64/ ./
  PATHS "${PROJECT_SOURCE_DIR}/../lua/"
  "${PROJECT_SOURCE_DIR}/extlibs/lua/"
  ${LUA_ROOT}
  $ENV{LUA_ROOT}
  /usr/local/
  /usr/
  /sw/
  /opt/local/
  /opt/csw/
  /opt/
  /usr/lib/x86_64-linux-gnu
)

message("\nFound Lua library at: ${LUA_LIBRARY}.\n")

IF(LUA_LIBRARY AND LUA_INCLUDE_DIR)
  SET(LUA_LIBRARIES ${LUA_LIBRARY})
  SET(LUA_FOUND TRUE)
ELSE(LUA_LIBRARY AND LUA_INCLUDE_DIR)
  SET(LUA_FOUND FALSE)
ENDIF(LUA_LIBRARY AND LUA_INCLUDE_DIR)

IF(LUA_FOUND)
  MESSAGE(STATUS "Found LUA in ${LUA_INCLUDE_DIR}")
ELSE(LUA_FOUND)
  IF(LUA_FIND_REQUIRED)
  MESSAGE(FATAL_ERROR "Could not find LUA library")
  ENDIF(LUA_FIND_REQUIRED)
ENDIF(LUA_FOUND)

MARK_AS_ADVANCED(
  LUA_LIBRARY
  LUA_INCLUDE_DIR
)

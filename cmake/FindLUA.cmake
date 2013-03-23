# -*- cmake -*-

# - Find LUA
# Find the LUA includes and library
# This module defines
#  LUA_INCLUDE_DIR, where to find lua.hpp
#  LUA_LIBRARIES, the libraries needed to use LUA.
#  LUA_FOUND, If false, do not try to use LUA.
#  also defined, but not for general use are
#  LUA_LIBRARY, where to find the LUA library.

FIND_PATH(LUA_INCLUDE_DIR lua5.1/lua.hpp
  PATHS
  /usr/local/include
  /usr/include
  /usr/include/lua
  /usr/local/include/lua
)

FIND_LIBRARY(LUA_LIBRARY
  NAMES liblua5.1 lua5.1 lua liblua
  PATHS /usr/lib /usr/local/lib /lib /lib64
  )

IF (LUA_LIBRARY AND LUA_INCLUDE_DIR)
    SET(LUA_LIBRARIES ${LUA_LIBRARY})
    SET(LUA_FOUND "YES")
ELSE (LUA_LIBRARY AND LUA_INCLUDE_DIR)
  SET(LUA_FOUND "NO")
ENDIF (LUA_LIBRARY AND LUA_INCLUDE_DIR)


IF (LUA_FOUND)
   IF (NOT LUA_FIND_QUIETLY)
      MESSAGE(STATUS "Found LUA in ${LUA_INCLUDE_DIR}")
   ENDIF (NOT LUA_FIND_QUIETLY)
ELSE (LUA_FOUND)
   IF (LUA_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find LUA library")
   ENDIF (LUA_FIND_REQUIRED)
ENDIF (LUA_FOUND)

# Deprecated declarations.
SET (NATIVE_LUA_INCLUDE_PATH ${LUA_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_LUA_LIB_PATH ${LUA_LIBRARY} PATH)

MARK_AS_ADVANCED(
  LUA_LIBRARY
  LUA_INCLUDE_DIR
  )

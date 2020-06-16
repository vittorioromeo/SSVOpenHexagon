FIND_PATH(ZLIB_INCLUDE_DIR ./zlib.h
  PATH_SUFFIXES include/ zlib/ zlib/include/ ./
  PATHS "${PROJECT_SOURCE_DIR}/../zlib/"
  "${PROJECT_SOURCE_DIR}/extlibs/zlib/"
  ${ZLIB_ROOT}
  $ENV{ZLIB_ROOT}
  /usr/local/
  /usr/
  /sw/
  /opt/local/
  /opt/csw/
  /opt/
)

message("\nFound Lua include at: ${ZLIB_INCLUDE_DIR}.\n")

FIND_LIBRARY(ZLIB_LIBRARY
  NAMES zlib.lib zlib libzlib zdll zlib1
  PATH_SUFFIXES lib/ lib/x86_64-linux-gnu/ lib64/ lua/ lua/lib/ lua/lib64/ ./
  PATHS "${PROJECT_SOURCE_DIR}/../zlib/"
  "${PROJECT_SOURCE_DIR}/extlibs/zlib/"
  ${ZLIB_ROOT}
  $ENV{ZLIB_ROOT}
  /usr/local/
  /usr/
  /sw/
  /opt/local/
  /opt/csw/
  /opt/
  /usr/lib/x86_64-linux-gnu
)

message("\nFound ZLIB library at: ${ZLIB_LIBRARY}.\n")

IF(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)
  SET(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
  SET(ZLIB_FOUND TRUE)
ELSE(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)
  SET(ZLIB_FOUND FALSE)
ENDIF(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)

IF(ZLIB_FOUND)
  MESSAGE(STATUS "Found ZLIB in ${ZLIB_INCLUDE_DIR}")
ELSE(ZLIB_FOUND)
  IF(ZLIB_FIND_REQUIRED)
  MESSAGE(FATAL_ERROR "Could not find ZLIB library")
  ENDIF(ZLIB_FIND_REQUIRED)
ENDIF(ZLIB_FOUND)

MARK_AS_ADVANCED(
  ZLIB_LIBRARY
  ZLIB_INCLUDE_DIR
)

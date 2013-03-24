# -*- cmake -*-

# - Find JSONCpp
# Find the JSONCpp includes and library
# This module defines
#  JSONCPP_INCLUDE_DIR, where to find json.h, etc.
#  JSONCPP_LIBRARIES, the libraries needed to use jsoncpp.
#  JSONCPP_FOUND, If false, do not try to use jsoncpp.
#  also defined, but not for general use are
#  JSONCPP_LIBRARY, where to find the jsoncpp library.

FIND_PATH(JSONCPP_INCLUDE_DIR 
  NAMES jsoncpp/json.h
  PATH_SUFFIXES include/
  PATHS "${PROJECT_SOURCE_DIR}/../jsoncpp/"
  "${PROJECT_SOURCE_DIR}/extlibs/jsoncpp/"
  ${SSVENTITYSYSTEM_ROOT}
  $ENV{SSVENTITYSYSTEM_ROOT}
  /usr/local/
  /usr/
  /sw/         # Fink
  /opt/local/  # DarwinPorts
  /opt/csw/    # Blastwave
  /opt/
)

message("\nFound JsonCpp include at: ${JSONCPP_INCLUDE_DIR}.\n")

# Get the GCC compiler version
EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
            ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
            OUTPUT_VARIABLE _gcc_COMPILER_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

SET(JSONCPP_NAMES ${JSONCPP_NAMES} libjson_linux-gcc-${_gcc_COMPILER_VERSION}_libmt.so)

IF (WIN32)
    SET(JSONCPP_NAMES ${JSONCPP_NAMES} libjson_mingw_libmt.a)
ENDIF ()

FIND_LIBRARY(JSONCPP_LIBRARY
  NAMES ${JSONCPP_NAMES}
  PATH_SUFFIXES lib/ lib64/ libs/ libs/mingw/
  PATHS "${PROJECT_SOURCE_DIR}/../jsoncpp/"
  "${PROJECT_SOURCE_DIR}/extlibs/jsoncpp/"
  ${SSVENTITYSYSTEM_ROOT}
  $ENV{SSVENTITYSYSTEM_ROOT}
  /usr/local/
  /usr/
  /sw/         # Fink
  /opt/local/  # DarwinPorts
  /opt/csw/    # Blastwave
  /opt/
)

message("\nFound JsonCpp library at: ${JSONCPP_LIBRARY}.\n")

IF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
    SET(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
    SET(JSONCPP_FOUND "YES")
ELSE (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
    SET(JSONCPP_FOUND "NO")
ENDIF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)

IF (JSONCPP_FOUND)
      MESSAGE(STATUS "Found JSONCpp in ${JSONCPP_INCLUDE_DIR}")
ELSE (JSONCPP_FOUND)
   IF (JSONCPP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find JSONCpp library")
   ENDIF (JSONCPP_FIND_REQUIRED)
ENDIF (JSONCPP_FOUND)

# Deprecated declarations.
SET (NATIVE_JSONCPP_INCLUDE_PATH ${JSONCPP_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_JSONCPP_LIB_PATH ${JSONCPP_LIBRARY} PATH)

MARK_AS_ADVANCED(
  JSONCPP_LIBRARY
  JSONCPP_INCLUDE_DIR
)

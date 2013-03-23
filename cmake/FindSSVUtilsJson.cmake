# -*- cmake -*-

# - Find SSVUtilsJson
# Find the SSVUtilsJson includes and library
# This module defines
# SSVUTILSJSON_INCLUDE_DIR, where to find SSVUtilsJson headers.
# SSVUTILSJSON_LIBRARIES, the libraries needed to use SSVUtilsJson.
# SSVUTILSJSON_FOUND, If false, do not try to use SSVUtilsJson.
# also defined, but not for general use are
# SSVUTILSJSON_LIBRARY, where to find the SSVUtilsJson library.

message("\nAttempting to find SSVUtilsJson.\n")

FIND_PATH(SSVUTILSJSON_INCLUDE_DIR
  NAMES SSVUtilsJson/SSVUtilsJson.h
  PATH_SUFFIXES include
  PATHS "/usr/local/"
)

message("\nFound SSVUtilsJson include at: ${SSVUTILSJSON_INCLUDE_DIR}.\n")

FIND_LIBRARY(SSVUTILSJSON_LIBRARY
  NAMES SSVUtilsJson libSSVUtilsJson SSVUtilsJson-s libSSVUtilsJson-s ssvutilsjson libssvutilsjson ssvutilsjson-s libssvutilsjson-s
  PATH_SUFFIXES lib/ lib64/
  PATHS /usr/ /usr/local/
)

message("\nFound SSVUtilsJson library at: ${SSVUTILSJSON_LIBRARY}.\n")

IF (SSVUTILSJSON_LIBRARY AND SSVUTILSJSON_INCLUDE_DIR)
    SET(SSVUTILSJSON_LIBRARIES ${SSVUTILSJSON_LIBRARY})
    SET(SSVUTILSJSON_FOUND TRUE)
ELSE (SSVUTILSJSON_LIBRARY AND SSVUTILSJSON_INCLUDE_DIR)
    SET(SSVUTILSJSON_FOUND FALSE)
ENDIF (SSVUTILSJSON_LIBRARY AND SSVUTILSJSON_INCLUDE_DIR)


IF (SSVUTILSJSON_FOUND)
      MESSAGE(STATUS "Found SSVUTILSJSON: ${SSVUTILSJSON_LIBRARIES}")
ELSE (SSVUTILSJSON_FOUND)
   IF (SSVUTILSJSON_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SSVUTILSJSON library")
   ENDIF (SSVUTILSJSON_FIND_REQUIRED)
ENDIF (SSVUTILSJSON_FOUND)

MARK_AS_ADVANCED(
  SSVUTILSJSON_LIBRARY
  SSVUTILSJSON_INCLUDE_DIR
)

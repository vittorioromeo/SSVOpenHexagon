# -*- cmake -*-

# - Find SSVUtils
# Find the SSVUtils includes and library
# This module defines
# SSVUTILS_INCLUDE_DIR, where to find SSVUtils headers.
# SSVUTILS_LIBRARIES, the libraries needed to use SSVUtils.
# SSVUTILS_FOUND, If false, do not try to use SSVUtils.
# also defined, but not for general use are
# SSVUTILS_LIBRARY, where to find the SSVUtils library.

message("\nAttempting to find SSVUtils.\n")

FIND_PATH(SSVUTILS_INCLUDE_DIR
  NAMES SSVUtils/SSVUtils.h
  PATH_SUFFIXES include
  PATHS "/usr/local/"
)

message("\nFound SSVUtils include at: ${SSVUTILS_INCLUDE_DIR}.\n")

FIND_LIBRARY(SSVUTILS_LIBRARY
  NAMES SSVUtils libSSVUtils SSVUtils-s libSSVUtils-s ssvutils libssvutils ssvutils-s libssvutils-s
  PATH_SUFFIXES lib/ lib64/
  PATHS /usr/ /usr/local/
)

message("\nFound SSVUtils library at: ${SSVUTILS_LIBRARY}.\n")

IF (SSVUTILS_LIBRARY AND SSVUTILS_INCLUDE_DIR)
    SET(SSVUTILS_LIBRARIES ${SSVUTILS_LIBRARY})
    SET(SSVUTILS_FOUND TRUE)
ELSE (SSVUTILS_LIBRARY AND SSVUTILS_INCLUDE_DIR)
    SET(SSVUTILS_FOUND FALSE)
ENDIF (SSVUTILS_LIBRARY AND SSVUTILS_INCLUDE_DIR)


IF (SSVUTILS_FOUND)
   #IF (NOT SSVUTILS_FIND_QUIETLY)
      MESSAGE(STATUS "Found SSVUTILS: ${SSVUTILS_LIBRARIES}")
   #ENDIF (NOT SSVUTILS_FIND_QUIETLY)
ELSE (SSVUTILS_FOUND)
   IF (SSVUTILS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SSVUTILS library")
   ENDIF (SSVUTILS_FIND_REQUIRED)
ENDIF (SSVUTILS_FOUND)

MARK_AS_ADVANCED(
  SSVUTILS_LIBRARY
  SSVUTILS_INCLUDE_DIR
)

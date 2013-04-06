EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE _gcc_COMPILER_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
SET(JSONCPP_NAMES ${JSONCPP_NAMES} libjson_linux-gcc-${_gcc_COMPILER_VERSION}_libmt.so)
IF(WIN32)
	SET(JSONCPP_NAMES ${JSONCPP_NAMES} libjson_mingw_libmt.a)
ENDIF()

set(FIND_JSONCPP_LIB_PATHS
	"${PROJECT_SOURCE_DIR}/../jsoncpp/"
	"${PROJECT_SOURCE_DIR}/extlibs/jsoncpp/"
	${JSONCPP_ROOT}
	$ENV{JSONCPP_ROOT}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr/
	/sw/
	/opt/local
	/opt/csw
	/opt
)

FIND_PATH(JSONCPP_INCLUDE_DIR
	NAMES jsoncpp/json.h
	PATH_SUFFIXES include/
	PATHS ${FIND_JSONCPP_LIB_PATHS}
)
message("\nFound JsonCpp include at: ${JSONCPP_INCLUDE_DIR}.\n")

FIND_LIBRARY(JSONCPP_LIBRARY
	NAMES ${JSONCPP_NAMES}
	PATH_SUFFIXES lib/ lib64/ libs/ libs/mingw/
	PATHS ${FIND_JSONCPP_LIB_PATHS}
)
message("\nFound JsonCpp library at: ${JSONCPP_LIBRARY}.\n")

IF(JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
	SET(JSONCPP_FOUND TRUE)
ELSE()
	SET(JSONCPP_FOUND FALSE)
ENDIF()

IF(JSONCPP_FOUND)
	MESSAGE(STATUS "Found JSONCpp in ${JSONCPP_INCLUDE_DIR}")
ELSE()
	IF(JSONCPP_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find JSONCpp library")
	ENDIF()
	set(JSONCPP_ROOT "" CACHE PATH "JsonCpp top-level directory")
	message("\n-> JsonCpp directory not found. Set SSVUTILS_ROOT to SSVUtils' top-level path (containing both \"include\" and \"lib\" directories).")
ENDIF()

MARK_AS_ADVANCED(
  JSONCPP_LIBRARY
  JSONCPP_INCLUDE_DIR
)

cmake_minimum_required (VERSION 2.6)

OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

project (icons)
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

set(icon_FILES 
		main.cpp 
		resource.rc
		)

SET_TARGET_PROPERTIES (icons PROPERTIES DEFINE_SYMBOL  "COMPILING_DLL" )
add_library(icons SHARED ${icon_FILES})


cmake_minimum_required (VERSION 3.9)

# default: SDL, Arnold z80, Release

project (arnold)

set(SDL_VERSION "USE_SDL" CACHE STRING "SDL version to use")
set_property(CACHE SDL_VERSION PROPERTY STRINGS USE_SDL USE_SDL2) 

set(Z80_VERSION "USE_ARNZ80" CACHE STRING "Z80 emulation to use")
set_property(CACHE Z80_VERSION PROPERTY STRINGS USE_ARNZ80 USE_INKZ80) 

set(EMBED_SDL "NO" CACHE STRING "Embed SDL window inside wxWidgets window")
set_property(CACHE EMBED_SDL PROPERTY STRINGS YES NO)

#if (WIN32)
#	set(SDL_VERSION "USE_SDL2")
#endif(WIN32)

get_filename_component(ABSOLUTE_INSTALL_PATH "${PROJECT_SOURCE_DIR}/../exe/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}" ABSOLUTE)
MESSAGE("Install path ${ABSOLUTE_INSTALL_PATH}")

# set where to put mac bundle; without it the bundle goes into the arngui folder
set(CMAKE_INSTALL_PREFIX ${ABSOLUTE_INSTALL_PATH})

# set macOS deployment target so we compile for a range of macOS releases
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")

if (Z80_VERSION STREQUAL "USE_INKZ80")
	set(USE_INKZ80 "Yes")
endif(Z80_VERSION STREQUAL "USE_INKZ80")

if (Z80_VERSION STREQUAL "USE_ARNZ80")
	set(USE_INKZ80 "No")
endif(Z80_VERSION STREQUAL "USE_ARNZ80")

if (SDL_VERSION STREQUAL "USE_SDL2")
	set(USE_SDL2 "Yes")
endif(SDL_VERSION STREQUAL "USE_SDL2")

if (SDL_VERSION STREQUAL "USE_SDL")
	set(USE_SDL "Yes")
endif(SDL_VERSION STREQUAL "USE_SDL")

# if build type is not defined, assume release
if(CMAKE_BUILD_TYPE STREQUAL "")
  SET(CMAKE_BUILD_TYPE Release)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "")

if(CMAKE_BUILD_TYPE STREQUAL "None")
  SET(CMAKE_BUILD_TYPE Release)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "None")

# set where to put mac bundle; without it the bundle goes into the arngui folder
set(CMAKE_MACOSX_BUNDLE "${PROJECT_SOURCE_DIR}/../exe/${CMAKE_BUILD_TYPE}/mac")

# set where to put exe; without it exe goes into arngui folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../exe/${CMAKE_BUILD_TYPE}/arnold")

# where to find cmake module files
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic ")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic ")

	# if debug, turn on generate debug information for gcc compiler
	IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")
	ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic ")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic ")

	# if debug, turn on generate debug information for gcc compiler
	IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")
	ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  # using Visual Studio C++
endif()

# determine big/little endian and set define based on that
INCLUDE(TestBigEndian)

TEST_BIG_ENDIAN(BIGENDIAN)
IF(${BIGENDIAN})
	if(USE_INKZ80 MATCHES "Yes")
		ADD_DEFINITIONS(-DINKZ80_ARCH_BIGENDIAN)
   	endif(USE_INKZ80 MATCHES "Yes")
	MESSAGE("Big endian found.")
ELSE(${BIGENDIAN})
     ADD_DEFINITIONS(-DCPC_LSB_FIRST)
   	MESSAGE("Little endian found.")
ENDIF(${BIGENDIAN})

if (EMBED_SDL MATCHES "YES")
set(USE_GTK2 "No") 
if (UNIX)
FIND_PACKAGE(GTK2)
if (GTK2_FOUND)
 MESSAGE("Using GTK 2\n")
	FIND_PACKAGE(GDK REQUIRED)
	if (GDK_FOUND)
		MESSAGE("Using GDK")
			set(USE_GTK2 "Yes")
		      SET(CMAKE_C_FLAGS "${GTK2_CFLAGS} ")
				STRING(REGEX REPLACE "-l" "" GTK_LIBRARIES "${GTK2_LINK_FLAGS} ")
				SEPARATE_ARGUMENTS(GTK_LIBRARIES)
			
			INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
			INCLUDE_DIRECTORIES(${GDK_INCLUDE_DIRS})
			ADD_DEFINITIONS(-DGTK2_EMBED_WINDOW)
	else(GDK_FOUND)
		MESSAGE("Not using GDK")
	endif(GDK_FOUND)
else (GTK2_FOUND)
 MESSAGE("Not using GTK2")
 endif(GTK2_FOUND)
endif(UNIX)
endif(EMBED_SDL MATCHES "YES")

if (WIN32)
	ADD_DEFINITIONS(-DWIN_EMBED_WINDOW)
endif(WIN32)

if (APPLE)
IF(USE_SDL2 MATCHES "Yes")
	ADD_DEFINITIONS(-DMAC_EMBED_WINDOW)
endif(USE_SDL2 MATCHES "Yes")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ ")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ ")
endif(APPLE)

# if we are using inkz80 z80 core set the define
if(USE_INKZ80 MATCHES "Yes")
     ADD_DEFINITIONS(-DINKZ80)
	MESSAGE("Will use InkZ80 Z80 emulation")
else(USE_INKZ80 MATCHES "Yes")
	MESSAGE("Will use Arnold's Z80 emulation")
endif(USE_INKZ80 MATCHES "Yes")

IF(USE_SDL MATCHES "Yes")
	MESSAGE("Will use SDL")
endif(USE_SDL MATCHES "Yes")

IF(USE_SDL2 MATCHES "Yes")
	MESSAGE("Will use SDL2")
endif(USE_SDL2 MATCHES "Yes")

# if we requested SDL2 try and find it and set it up
# error if we didn't find it.
IF(USE_SDL2 MATCHES "Yes")

FIND_LIBRARY(SDL2_LIBRARY SDL2)
FIND_PACKAGE(SDL2 REQUIRED)
    if ( SDL2_FOUND )
      MESSAGE("SDL2 found\n")
      INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIR})
      ADD_DEFINITIONS(-DUSE_SDL2)
      SET(USING_SDL2 "Yes")
    else (SDL2_FOUND)
      message ( FATAL_ERROR "SDL2 not found!" )
    endif(SDL2_FOUND)
endif(USE_SDL2 MATCHES "Yes")

# if sdl2 was found, use it. Otherwise assume SDL
if ( SDL2_FOUND )
else (SDL2_FOUND)
FIND_PACKAGE( SDL REQUIRED)
if (SDL_FOUND )
  MESSAGE("SDL found\n")
  ADD_DEFINITIONS(-DUSE_SDL)
  SET(USING_SDL "Yes")
else (SDL_FOUND)
   message ( FATAL_ERROR "SDL not found!" )
endif(SDL_FOUND)

endif(SDL2_FOUND)

if (APPLE)
INCLUDE(FindEXPAT)
FIND_PACKAGE(EXPAT REQUIRED)
IF(NOT EXPAT_FOUND)
    MESSAGE(FATAL_ERROR "EXPAT libs/dlls not found!\nInstall/Compile EXPAT.\n")
endif(NOT EXPAT_FOUND)
  
INCLUDE_DIRECTORIES(${EXPAT_INCLUDE_DIRS})
endif(APPLE)

# find wxwidgets
INCLUDE(FindwxWidgets)

# if debug build then enable wx widgets debug libraries
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	SET(wxWidgets_USE_DEBUG ON)
	MESSAGE("Using DEBUG version of wxWidgets")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -D__WXDEBUG__")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D__WXDEBUG__")
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

# if mac, statically link against wxwidgets. I can't find a
# wxwidgets framework
if(APPLE)
	SET(wxWidgets_USE_STATIC ON)
	
	MESSAGE("Using STATIC version of wxWidgets")
endif(APPLE)

# these are the packages we want to link against
# mac static link requires base html and xml
FIND_PACKAGE(wxWidgets REQUIRED core base adv xrc net base html xml)
  
# setup wxwidgets if found else error
  IF(NOT wxWidgets_FOUND)
    MESSAGE(FATAL_ERROR "wxWidgets 2.8+ libs/dlls not found!\nInstall/Compile wxWidget.\n")
  endif(NOT wxWidgets_FOUND)
  
 INCLUDE( ${wxWidgets_USE_FILE} )
  INCLUDE_DIRECTORIES( 
      ${WXWINDOWS_INCLUDE_PATH}  
      )

# add cpc directory
add_subdirectory (cpc)

# if inkz80, include it's directory and link against it
IF(USE_INKZ80 MATCHES "Yes")
add_subdirectory (inkz80)
endif(USE_INKZ80 MATCHES "Yes")

# add gui librray
add_subdirectory (arngui) 



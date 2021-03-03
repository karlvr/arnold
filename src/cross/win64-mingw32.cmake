# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

add_definitions(-DWIN32)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/bin /opt/SDL-1.2.13 /usr/local/i586-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM 	BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
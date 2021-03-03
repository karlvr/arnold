cmake_minimum_required (VERSION 2.6)
project (makez80)

set(makez80_FILES .c)

add_executable(makez80 WIN32 makez80.c z80gen.c)

#!/bin/bash
# Make Arnold using updated Arnold z80 core
# Debug version with symbols and no optimisations
set -eu
./clean.sh
BUILDDIR=build_arnz80_sdl2_debug
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
${CMAKE} -DZ80_VERSION:String="USE_ARNZ80"  -DSDL_VERSION:String="USE_SDL2" -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" ..
make

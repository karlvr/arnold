#!/bin/bash
# Make Arnold using updated Arnold z80 core
# Debug version with symbols and no optimisations
set -eu
./clean.sh
BUILDDIR=build_arnz80_sdl2_debug
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
if [ "$(uname)" == "Darwin" ]; then
	CMAKE=/Applications/CMake.app/Contents/bin/cMake
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	CMAKE=cmake
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
	CMAKE=cmake
fi
${CMAKE} -DZ80_VERSION:String="USE_ARNZ80"  -DSDL_VERSION:String="USE_SDL2" -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" ..
make

#!/bin/bash
# Make Arnold with updated Arnold z80 core
# Release = optimised and stripped version
set -eu
./clean.sh
BUILDDIR=build_arnz80_release_embed
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
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
${CMAKE} -DEMBED_SDL:String="YES" -DZ80_VERSION:String="USE_ARNZ80" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
make

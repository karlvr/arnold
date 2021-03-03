#!/bin/bash
# Make Arnold with INKZ80 Z80 Emulation (by Mark Incley).
# Release build (no symbols and optimisations turned on)
set -eu
./clean.sh
BUILDDIR=build_inkz80_release
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
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
#make

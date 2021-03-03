#!/bin/bash
# Make Arnold with INKZ80 Z80 Emulation Core (by Mark Incley)
# Debug version with symbols and no optimisations
set -eu
./clean.sh
BUILDDIR=build_inkz80_debug
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
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" ..
make

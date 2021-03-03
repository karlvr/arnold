#!/bin/bash
# Make Arnold with INKZ80 Z80 Emulation Core (by Mark Incley)
# Debug version with symbols and no optimisations
set -eu
./clean.sh
BUILDDIR=build_inkz80_debug
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" ..
make

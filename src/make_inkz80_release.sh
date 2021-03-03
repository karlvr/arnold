#!/bin/bash
# Make Arnold with INKZ80 Z80 Emulation (by Mark Incley).
# Release build (no symbols and optimisations turned on)
set -eu
./clean.sh
BUILDDIR=build_inkz80_release
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
make

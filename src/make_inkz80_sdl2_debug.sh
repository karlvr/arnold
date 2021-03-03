#!/bin/bash
# Make Arnold with INKZ80 Z80 Emulation Core (by Mark Incley)
# Debug version with symbols and no optimisations
set -eu
# sudo cp /usr/lib/wx/config/gtk2-unicode-debug-2.8 /usr/bin/wx-config
./clean.sh
BUILDDIR=build_inkz80_sdl2_debug
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DSDL_VERSION:String="USE_SDL2" -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" ..
make

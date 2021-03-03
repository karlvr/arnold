#!/bin/bash
# Make Arnold with INKZ80 Z80 Emulation (by Mark Incley).
# Release build (no symbols and optimisations turned on)
ln -s /usr/lib/wx/config/gtk2-unicode-release-2.8 /usr/bin/wx-config
set -eu
./clean.sh
BUILDDIR=build_inkz80_sdl2_release
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DSDL_VERSION:String="USE_SDL2" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
make

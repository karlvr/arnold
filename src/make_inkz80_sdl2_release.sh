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
if [ "$(uname)" == "Darwin" ]; then
	CMAKE=/Applications/CMake.app/Contents/bin/cMake
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	CMAKE=cmake
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
	CMAKE=cmake
fi
${CMAKE} -DZ80_VERSION:String="USE_INKZ80" -DSDL_VERSION:String="USE_SDL2" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
make

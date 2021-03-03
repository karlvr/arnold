#!/bin/bash
# Make Arnold with updated Arnold z80 core
# Release = optimised and stripped version
set -eu
./clean.sh
BUILDDIR=build_arnz80_sdl2_release_embed
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
${CMAKE} -DEMBED_SDL:String="YES" -DZ80_VERSION:String="USE_ARNZ80"  -DSDL_VERSION:String="USE_SDL2" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
make

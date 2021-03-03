#!/bin/bash
# Make Arnold with updated Arnold z80 core
# Release = optimised and stripped version
set -eu
./clean.sh
BUILDDIR=build_arnz80_release
rm -rf ${BUILDDIR}
mkdir ${BUILDDIR}
cd ${BUILDDIR}
CMAKE=cmake
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
${CMAKE} -DZ80_VERSION:String="USE_ARNZ80" -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" ..
make

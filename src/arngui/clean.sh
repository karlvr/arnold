#!/bin/bash
set -eu
rm -f arnold
rm -f *.bak
rm -f arnold_debug
rm -f arnold.exe
rm -f arnold_debug.exe
rm -f Makefile
rm -f cmake_install.cmake
rm -rf CMakeFiles
rm -f CMakeCache.txt
rm -f *-baseline
rm -f *-merge
rm -f *-original
rm -f *-output
rm -f *.bak
rm -f *.sdf
rm -f *.suo
rm -f *.user
pushd arngui && ./clean.sh && popd
pushd sdl && ./clean.sh && popd
pushd sdl2 && ./clean.sh && popd
pushd sdlcommon && ./clean.sh && popd

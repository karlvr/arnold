#!/bin/bash
set -eu
rm -f Makefile
rm -rf build
rm -rf Arnold
rm -rf build_*
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
pushd cpc && ./clean.sh && popd
pushd cpc/debugger && ./clean.sh && popd
pushd cpc/z80 && ./clean.sh && popd
pushd cpc/z80tools && ./clean.sh && popd
pushd cpc/diskimage && ./clean.sh && popd
pushd tools/makez80 && ./clean.sh && popd
pushd inkz80 && ./clean.sh && popd
pushd test/psg/psg && ./clean.sh && popd
pushd test/monitor && ./clean.sh && popd

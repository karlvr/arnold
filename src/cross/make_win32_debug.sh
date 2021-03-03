#!/bin/bash
set -eu
./clean.sh
cd build && rm -rf *
cmake -DCMAKE_TOOLCHAIN_FILE=./win32-mingw32.cmake -DCMAKE_BUILD_DIR:STRING=build -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" 
make
cp arngui/arnold_debug.exe ../exe/arnold/windows/x86/debug
rm arngui/arnold_debug.exe

#!/bin/bash
set -eu
./clean.sh
cd build && rm -rf *
cmake -DBUILD64BIT=true -DCMAKE_TOOLCHAIN_FILE=./win64-mingw32.cmake -DCMAKE_BUILD_DIR:STRING=build -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" 
make
cp arngui/arnold_debug.exe ../exe/arnold/windows/x64/debug
rm arngui/arnold_debug.exe

#!/bin/bash
set -eu
cd ..
./clean.sh
pushd build && rm -rf * && popd
cmake -DUSE_INKZ80:String="Yes" -DCMAKE_TOOLCHAIN_FILE=cross/win32-mingw32.cmake -DCMAKE_BUILD_DIR:STRING=build -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" 
make
cp arngui/arnold.exe ../exe/arnold/windows/x86/release
rm arngui/arnold.exe

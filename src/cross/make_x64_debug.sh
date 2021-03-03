#!/bin/bash
set -eu
./clean.sh
cd build && rm -rf *
cmake -DCMAKE_BUILD_DIR:STRING=build -DBUILD64BIT -DCMAKE_BUILD_TYPE:STRING=Debug -G "Unix Makefiles" 
make
cp arngui/arnold_debug ../exe/arnold/linux/x86/debug
rm arngui/arnold_debug

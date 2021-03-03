#!/bin/bash
set -eu
./clean.sh
cd build && rm -rf *
cmake -DCMAKE_BUILD_DIR:STRING=build -DBUILD64BIT -DCMAKE_BUILD_TYPE:STRING=Release -G "Unix Makefiles" 
make
cp arngui/arnold ../exe/arnold/linux/x86/release
rm arngui/arnold

#!/bin/bash
set -eu
pushd arngui
./make_xrc.sh
./copy_linux.sh
./copy_windows.sh
popd
./make_inkz80_debug.sh
./make_inkz80_release.sh

# ./make_x86_debug.sh
# ./make_x86_release.sh
# ./make_x64_debug.sh
# ./make_x64_release.sh
# ./make_win32_debug.sh
# ./make_win32_release.sh
# ./make_win64_debug.sh
# ./make_win64_release.sh

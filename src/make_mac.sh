#!/bin/bash
# Make Arnold using updated Arnold z80 core
# Debug version with symbols and no optimisations
set -eu
./make_arnz80_sdl2_release.sh
cd build_arnz80_sdl2_release
make install

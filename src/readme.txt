debug has debugging symbols and no optimisations
release has no symbols and maximum optimisations

archive.sh - cleans the directory and uses 7-zip to make an archive of the src

makeit.sh - performs all the work needed (generation of xrc, setup of exe, building debug and release versions)

inkz80 versions use inkz80 a Z80 emulation by my friend Mark Incley. This has correct z80 flags and instruction emulation
timing is not correct for cpc at this time.

arnold's z80 core is the original z80 core with some fixes and correct timings, z80 flags emulation is not perfect.

make_inkz80_debug.sh - compile for "native" linux (builds for your cpu) using INKZ80 Z80 Emulation Core
make_inkz80_release.sh - compile for "native" linux (builds for your cpu) using INKZ80 Z80 Emulation Core

make_arnz80_debug.sh - compile for "native" linux (builds for your cpu) using Arnold's Z80 Emulation Core
make_arnz80_release.sh - compile for "native" linux (builds for your cpu) using Arnold's Z80 Emulation Core


SDL2 version is stable under windows, but not stable under linux.

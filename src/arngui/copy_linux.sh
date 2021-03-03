# copy data files to generate exe directories

#!/bin/bash
set -eu

mkdir -p ../../exe/arnold/linux/x86/debug
mkdir -p ../../exe/arnold/linux/x86/release
mkdir -p ../../exe/arnold/linux/x64/debug
mkdir -p ../../exe/arnold/linux/x64/release

cp rundebug.sh ../../exe/arnold/linux/x86/debug/run.sh
cp run.sh ../../exe/arnold/linux/x86/release
cp rundebug.sh ../../exe/arnold/linux/x64/debug/run.sh
cp run.sh ../../exe/arnold/linux/x64/release

cp breakpoint.png ../../exe/arnold/linux/x86/debug
cp breakpoint.png ../../exe/arnold/linux/x86/release
cp breakpoint.png ../../exe/arnold/linux/x64/debug
cp breakpoint.png ../../exe/arnold/linux/x64/release

cp icons/32x32/*.png ../../exe/arnold/linux/x86/debug
cp icons/32x32/*.png ../../exe/arnold/linux/x86/release
cp icons/32x32/*.png ../../exe/arnold/linux/x64/debug
cp icons/32x32/*.png ../../exe/arnold/linux/x64/release

cp current.png ../../exe/arnold/linux/x86/debug
cp current.png ../../exe/arnold/linux/x86/release
cp current.png ../../exe/arnold/linux/x64/debug
cp current.png ../../exe/arnold/linux/x64/release

cp arnlogo.png ../../exe/arnold/linux/x86/debug
cp arnlogo.png ../../exe/arnold/linux/x86/release
cp arnlogo.png ../../exe/arnold/linux/x64/debug
cp arnlogo.png ../../exe/arnold/linux/x64/release

cp roms.zip ../../exe/arnold/linux/x86/debug
cp roms.zip ../../exe/arnold/linux/x86/release
cp roms.zip ../../exe/arnold/linux/x64/debug
cp roms.zip ../../exe/arnold/linux/x64/release

cp GUIFrame.xrc ../../exe/arnold/linux/x86/debug
cp GUIFrame.xrc ../../exe/arnold/linux/x86/release
cp GUIFrame.xrc ../../exe/arnold/linux/x64/debug
cp GUIFrame.xrc ../../exe/arnold/linux/x64/release

cp windows/arnold.ico ../../exe/arnold/linux/x86/debug/arnold.bmp
cp windows/arnold.ico ../../exe/arnold/linux/x86/release/arnold.bmp
cp windows/arnold.ico ../../exe/arnold/linux/x64/debug/arnold.bmp
cp windows/arnold.ico ../../exe/arnold/linux/x64/release/arnold.bmp

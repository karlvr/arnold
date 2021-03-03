rm -rf dmg
rm arnold.dmg
mkdir dmg
cp -r ../../exe/Release/arnold/arnold.app dmg
hdiutil create arnold.dmg -volname "Arnold emulator" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -imagekey zlib-level=9 -format UDZO -srcfolder "dmg"

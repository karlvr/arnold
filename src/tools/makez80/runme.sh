cmake -G "Unix Makefiles"
make
mkdir out
./makez80
cp out/z80.c ../../cpc/z80


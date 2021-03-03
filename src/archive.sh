#!/bin/bash
set -eu
rm -f src.7z
./clean.sh
7za a src.7z ./*


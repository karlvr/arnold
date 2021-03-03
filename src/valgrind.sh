#!/bin/bash
set -eu
cd ../exe/arnold/linux/x86/debug && valgrind --track-origins=yes --leak-check=full --show-reachable=yes --log-file=valgrind_report.txt ./arnold_debug

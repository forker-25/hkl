#!/bin/sh
CC=${CC:-x86_64-w64-mingw32-gcc}
$CC -O2 -s -static -mwindows -o hkl.exe src/main.c -ladvapi32 -luser32 -lkernel32 -lshell32 -lshlwapi -lole32 -loleaut32 -luuid

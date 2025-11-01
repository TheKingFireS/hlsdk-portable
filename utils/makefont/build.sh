#!/usr/bin/env sh

# ext/ folder for cmdlib.h and wadlib.h to be used by makefont.cpp
# common/ folder for qfont.h to be used by makefont.cpp
# dlls/ folder for extdll.h to be used by makefont.cpp
# public/ folder for build.h to be used by extdll.h
# engine/ folder for progdefs.h to be used by extdll.h
exec gcc makefont.cpp -I ext/ -I ../../common/ -I ../../dlls/ -I ../../public/ -I ../../engine/ "$@"

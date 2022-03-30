@echo off
windres -F pe-x86-64 -o resources.o resources.rc
vmake -o mr.exe -x resources.o -L -luser32 -L -lcomctl32 -L -lshell32 -L -lgdi32 -L -ld2d1 clang++ -O2 -std=c++20 -Wall -Wextra
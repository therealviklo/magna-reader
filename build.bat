@echo off
windres -F pe-x86-64 -o resources.o resources.rc
flags.bat clang++ NAMN resources.o
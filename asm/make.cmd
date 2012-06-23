@echo off
for /R %%i IN (*.S) DO E:\devkitPro\devkitPPC\bin\powerpc-eabi-as.exe %%i -o %%~ni.elf
for /R %%i IN (*.S) DO E:\devkitPro\devkitPPC\bin\powerpc-eabi-strip.exe -s %%~ni.elf -O binary -o %%~ni.bin
for /R %%i in (*.bin) DO "C:\Users\crediar\Documents\Visual Studio 2010\Projects\bin2h\Release\bin2h.exe" %%~ni.bin

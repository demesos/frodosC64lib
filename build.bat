@REM Script for compiling the file given in the argument or the
@REM the most recently changed c file in the folder for the c64 target
@REM cc65 must be in the path
@REM Sorry for it being a Windows file, I'm ashamed not using make

IF [%1]==[] (
for /f  "delims=" %%f in ('dir /b /od *.c') do (
set file=%%f
set purefile=%%~nf
)
) ELSE (
set file=%1%
set purefile=%~n1
)

rem cl65 -c -t c64 %file% -o %purefile%.o
rem cl65 -c -t c64  %~dp0/musicplayer.s -o musicplayer.o
rem cl65 -t c64 -C %~dp0/c64-himem-c0.cfg -O -o %purefile%.prg %~dp0/musicplayer.o %purefile%.o
cl65 -C %~dp0/c64-himem-c0.cfg -O -o %purefile%.prg -t c64 %file%
@pause
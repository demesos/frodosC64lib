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

cl65 -O -o %purefile%.prg -t c64 %file%
@pause
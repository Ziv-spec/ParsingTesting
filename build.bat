@echo off

IF NOT EXIST "build" mkdir build
pushd build

REM ONLY IF I COMPILE IN "C"
SET CompilerFlags_C=/TP

SET WARNINGS=-WX -W4 -wd4021 -wd4100
SET CompilerFlags=-MT -nologo -Gm- -GR- -EHa- -FC -Z7 -Oi  %CompilerFlags_C%

cl %WARNIGS% %CompilerFlags% -DDEBUG=1 ..\json_parser_example_program.c  

rem cl %WARNIGS% %CompilerFlags% ..\tests.c  
rem tests.exe

popd

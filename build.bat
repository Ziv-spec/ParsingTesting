@echo off

IF NOT EXIST "build" mkdir build
pushd build

REM ONLY IF I COMPILE IN "C"
SET CompilerFlags_C=/TP

SET WARNINGS=-WX -W4 -wd4021 -wd4100
SET CompilerFlags=-MT -nologo -Gm- -GR- -EHa- -FC -Z7 -Oi -DEBUG=0 %CompilerFlags_C%

cl %WARNIGS% %CompilerFlags% ..\json_parser_example_program.c  

popd

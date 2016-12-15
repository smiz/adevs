@echo off
rem Clean up stale files
rmdir /S /Q HelloWorld
del /F *.exe *.obj
rem Build the FMU 
omc sim_hello.mos
rem Unpack the fmu data 
"C:\Program Files\7-Zip\7z" x -oHelloWorld HelloWorld.fmu
rem Cleanup the junk produced by the omc compiler
del /F *fmu*
del /F *.libs
del /F *.json
rem Generate the HelloWorld.h header file
python ..\..\..\util\xml2cpp.py -o HelloWorld -type double -f HelloWorld\\binaries\\win64\\HelloWorld.dll -r HelloWorld\modelDescription.xml
rem Compile our simulator. You must have in the include path
rem    The adevs include directory
rem    The FMI for model exchange header files
rem You must link to system library libdl
set HOME=C:\Users\home426\Documents
set FMI=%HOME%\FMI_for_ModelExchange_and_CoSimulation_v2.0
set ADEVS=%HOME%\adevs-code
cl /nologo /O2 /GR /MT /EHsc /I%FMI% /I%ADEVS%\include main_hello.cpp 
rem Run the model
main_hello


@echo off
rem Clean up stale files
rmdir /S /Q Sampled
rmdir /S /Q Quantized
rmdir /S /Q Control
del /F *.exe *.obj
rem Build each FMU and cleanup after it 
omc sim_control.mos
"C:\Program Files\7-Zip\7z" x -oControl Control.fmu
del /F *fmu*
del /F *.libs
del /F *.json
python ..\..\..\util\xml2cpp.py -o Control -type IO_Type -f Control\\binaries\\win64\\Control.dll -r Control\modelDescription.xml
omc sim_quantized.mos
"C:\Program Files\7-Zip\7z" x -oQuantized Robot.fmu
del /F *fmu*
del /F *.libs
del /F *.json
omc sim_sampled.mos
"C:\Program Files\7-Zip\7z" x -oSampled Robot.fmu
del /F *fmu*
del /F *.libs
del /F *.json
rem Set include paths
set HOME=C:\Users\home426\Documents
set FMI=%HOME%\FMI_for_ModelExchange_and_CoSimulation_v2.0
set ADEVS=%HOME%\adevs-code
set SRC=Ethernet.cpp main.cpp
rem Generate the sampled data simulation
python ..\..\..\util\xml2cpp.py -o Robot -type IO_Type -f Sampled\\binaries\\win64\\Robot.dll -r Sampled\modelDescription.xml
cl /nologo /O2 /GR /MT /EHsc /I%FMI% /I%ADEVS%\include /Fesampled %SRC% %ADEVS%\src\adevs.lib
rem Generate the quantized data simulation
python ..\..\..\util\xml2cpp.py -o Robot -type IO_Type -f Quantized\\binaries\\win64\\Robot.dll -r Quantized\modelDescription.xml
cl /nologo /O2 /GR /MT /EHsc /I%FMI% /I%ADEVS%\include /Fequantized %SRC% %ADEVS%\src\adevs.lib

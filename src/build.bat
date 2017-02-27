@echo off
cl /nologo /O2 /GR /MT /EHsc /I..\include /c poly.cpp 
cl /nologo /O2 /GR /MT /EHsc /I..\include /c rv.cpp 
lib /VERBOSE /OUT:adevs.lib *.obj
del *.obj
echo Built static library adevs.lib

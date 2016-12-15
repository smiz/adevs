@echo off
cl /nologo /O2 /GR /MT /EHsc /I..\include /c poly.cpp 
cl /nologo /O2 /GR /MT /EHsc /I..\include /c rv.cpp 
cl /nologo /O2 /GR /MT /EHsc /I..\include /c time.cpp 
lib /VERBOSE /OUT:adevs.lib *.obj
del *.obj
echo Built static library adevs.lib

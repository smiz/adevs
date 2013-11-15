@echo off
cl /O2 /GR /MT /EHsc /I..\include /c poly.cpp 
cl /O2 /GR /MT /EHsc /I..\include /c rv.cpp 
cl /O2 /GR /MT /EHsc /I..\include /c time.cpp 
lib /VERBOSE /OUT:adevs.lib *.obj
del *.obj
echo Built static library adevs.lib

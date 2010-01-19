@echo off
cl /GR /MT /EHsc /I%JAVA_HOME%\include /I%JAVA_HOME%\include\win32 /I..\..\include /I.. /LD /Fejava_adevs.dll JavaAtomic.cpp JavaSimulator.cpp JavaDevs.cpp JavaEventListenerManager.cpp JavaNetwork.cpp 
javac adevs/*.java
jar cfv adevs.jar adevs/*.class
del adevs\*.class
del *.obj
echo jar file: adevs.jar, dll: java_adevs.dll, import lib: java_adevs.lib"

#
# Makefile for the GNU g++ compiler.  Use make to build the adevs library with
# this compiler.
#
# Compiler flags
#CFLAGS = -fPIC -fopenmp -Wall -I../../include -I../
# Best bet for GNU compiler
# CC = g++
CC = cl
# OPTFLAG = -g
# For java
OBJS = JavvAtomic.obj JavaSimulator.obj
CFLAGS += -I${JAVA_HOME}/include -I${JAVA_HOME}/include/win32
TARGET = libjava_adevs.dll
#
# SHOULD NOT NEED TO EDIT BELOW THIS LINE
#

OBJS = JavaSimulator.o JavaAtomic.o

.SUFFIXES: .cpp
.cpp.obj:
	${CC} ${CFLAGS} ${OPTFLAG} -c $<

jni: ${OBJS}
	${CC} -shared -Wl,-soname,java_adevs -o ${TARGET} ${OBJS}
	javac adevs/*.java
	jar cfv adevs.jar adevs/*.class

clean:
	rm -f *.o core *.a *.so *.jar

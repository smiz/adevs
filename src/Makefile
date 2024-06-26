#
# Makefile for the GNU g++ compiler. There are three make targets.
#
# 1) java_adevs
#
# This target builds the java language bindings to the adevs simulator.
# Point this at your java installation if you want to build the Java bindings.
JAVA_HOME = /usr/local/jdk-15.0.2
java_adevs: CFLAGS += -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux 

#
# 2) adevs
#
# This builds the vanilla adevs library against which to link your c++ 
# simulations.
#

# These are generic options for the GNU C++ compiler
# or clang++ if you change CXX=clag++ and CC=clang

# Compiler flags
CFLAGS += -fPIC -Wall -I../include -I./ 
# Best bet for GNU compiler
CXX = g++
CC = gcc
OPTFLAG = -O3 

#
# SHOULD NOT NEED TO EDIT BELOW THIS LINE
#

ADEVS_OBJS = \
	spline.o \
	rv.o \
	poly.o 

JAVA_OBJS = \
	adevs_jni/JavaAtomic.o \
	adevs_jni/JavaDevs.o \
	adevs_jni/JavaEventListenerManager.o \
	adevs_jni/JavaNetwork.o \
	adevs_jni/JavaSimulator.o

.SUFFIXES: .cpp .c
.cpp.o:
	${CXX} ${CFLAGS} ${OPTFLAG} -o $@ -c $<
.c.o:
	${CC} ${CFLAGS} ${OPTFLAG} -o $@ -c $<

# This builds the static link library libadevs.a for linking
# to C++ simulations
adevs: ${ADEVS_OBJS} 
	ar -r libadevs.a ${ADEVS_OBJS}
	${CXX} -shared -Wl,-soname,libadevs.so.3 -o libadevs.so.3.0 ${ADEVS_OBJS}

# This builds the Java bindings to the simulator
java_adevs: ${JAVA_OBJS}
	${CXX} ${OMP} ${OPTFLAG} -shared -Wl,-soname,java_adevs -o libjava_adevs.so ${JAVA_OBJS}
	javac -Xlint:deprecation adevs_jni/adevs/*.java
	cd adevs_jni ; jar cfv ../adevs.jar adevs/*.class

install:
	mkdir -p "${DESTDIR}"
	install -m 755 -p libadevs.so.3.0 "${DESTDIR}"
	ln -f -s libadevs.so.3.0 "${DESTDIR}/libadevs.so.3"
	ln -f -s libadevs.so.3.0 "${DESTDIR}/libadevs.so"

clean:
	rm -f *.o core *.a *.so adevs_jni/*.o adevs_jni/adevs/*.class *.jar rm libadevs.so.* 

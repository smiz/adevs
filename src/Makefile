#
# Makefile for the GNU g++ compiler. There are three make targets.
#
# 1) java_adevs
#
# This target builds the java language bindings to the adevs simulator.
# Point this at your java installation if you want to build the Java bindings.
JAVA_HOME = /usr/local/jdk1.7.0_17
java_adevs: CFLAGS += -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux 

#
# 2) adevs
#
# This builds the vanilla adevs library against which to link your c++ 
# simulations.
#
# 3) adevs_with_modelica
#
# This builds adevs as above and adds to the vanilla library some
# additional object files that are needed to use adevs models 
# produced by the Open Modelica compiler. To build this this target
# you need to have compiled a recent version of the omc compiler
# and have the object files from that available for linking into the
# adevs object library. Point MODELICA_HOME at your Open Modelica 
# installation if you want to build the Modelica extensions for adevs.
SUNDIALS_HOME = /usr/local/sundials
MODELICA_HOME = ${HOME}/Code/openmodelica/trunk
adevs_with_modelica: CFLAGS += -I${MODELICA_HOME}/build/include/omc -I${SUNDIALS_HOME}/include 

include Makefile.modelica

# These are generic options for the GNU C++ compiler.

# Use OpenMP? 
OMP = -fopenmp
# Compiler flags
CFLAGS += ${OMP} -fPIC -Wall -I../include -I./
# Best bet for GNU compiler
CXX = g++
CC = gcc
OPTFLAG = -O3

#
# SHOULD NOT NEED TO EDIT BELOW THIS LINE
#

ADEVS_OBJS = \
	rv.o \
	poly.o \
	time.o

JAVA_OBJS = \
	adevs_jni/JavaAtomic.o \
	adevs_jni/JavaDevs.o \
	adevs_jni/JavaEventListenerManager.o \
	adevs_jni/JavaNetwork.o \
	adevs_jni/JavaSimulator.o

MODELICA_OBJS += modelica_runtime.o

.SUFFIXES: .cpp .c
.cpp.o:
	${CXX} ${CFLAGS} ${OPTFLAG} -o $@ -c $<
.c.o:
	${CC} ${CFLAGS} ${OPTFLAG} -o $@ -c $<

# This builds the static link library libadevs.a for linking
# to C++ simulations
adevs: ${ADEVS_OBJS} 
	ar -r libadevs.a ${ADEVS_OBJS}
	${CXX} -shared -Wl,-soname,libadevs.so.2 -o libadevs.so.2.6 ${ADEVS_OBJS}

# This adds the modelica extensions to the adevs library
adevs_with_modelica: ${ADEVS_OBJS} ${MODELICA_OBJS}
	ar -r libadevs.a ${ADEVS_OBJS} ${MODELICA_OBJS} 
	${CXX} -shared -Wl,-soname,libadevs.so.2 -o libadevs.so.2.6 ${ADEVS_OBJS} ${MODELICA_OBJS} 

# This builds the Java bindings to the simulator
java_adevs: ${JAVA_OBJS}
	${CXX} ${OMP} -shared -Wl,-soname,java_adevs -o libjava_adevs.so ${JAVA_OBJS}
	javac adevs_jni/adevs/*.java
	cd adevs_jni ; jar cfv ../adevs.jar adevs/*.class

install:
	mkdir -p "${DESTDIR}"
	install -m 755 -p libadevs.so.2.6 "${DESTDIR}"
	ln -f -s libadevs.so.2.6 "${DESTDIR}/libadevs.so.2"
	ln -f -s libadevs.so.2.6 "${DESTDIR}/libadevs.so"

clean:
	rm -f *.o core *.a *.so adevs_jni/*.o adevs_jni/adevs/*.class *.jar rm libadevs.so.*

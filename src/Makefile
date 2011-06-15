#
# Makefile for the GNU g++ compiler.  Use make to build the adevs library with
# this compiler.
#
# Point this at your java installation if you want to build the Java bindings
JAVA_HOME = /usr/local/jdk1.6.0_21
# Use OpenMP? 
OMP = -fopenmp
# Compiler flags
CFLAGS = ${OMP} -fPIC -Wall -I../include -I./ -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux
# Best bet for GNU compiler
 CC = g++
 OPTFLAG = -O3

#
# SHOULD NOT NEED TO EDIT BELOW THIS LINE
#

ADEVS_OBJS = rv.o mtrand.o poly.o time.o
ADEVSM_OBJS = modelica/modelica_runtime.o 
JAVA_OBJS = \
	adevs_jni/JavaAtomic.o \
	adevs_jni/JavaDevs.o \
	adevs_jni/JavaEventListenerManager.o \
	adevs_jni/JavaNetwork.o \
	adevs_jni/JavaSimulator.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} -o $@ -c $<

# This builds the static link library libadevs.a for linking
# to C++ simulations
adevs: ${ADEVS_OBJS} ${ADEVSM_OBJS}
	ar -r libadevs.a ${ADEVS_OBJS}
	ar -r libadevsm.a ${ADEVSM_OBJS}

# This builds the Java bindings to the simulator
java_adevs: ${JAVA_OBJS}
	${CC} ${OMP} -shared -Wl,-soname,java_adevs -o libjava_adevs.so ${JAVA_OBJS}
	javac adevs_jni/adevs/*.java
	cd adevs_jni ; jar cfv ../adevs.jar adevs/*.class

clean:
	rm -f *.o core *.a *.so adevs_jni/*.o adevs_jni/adevs/*.class *.jar modelica/*.o

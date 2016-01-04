#
# Makefile for the GNU g++ compiler. There are three make targets.
#
# 1) java_adevs
#
# This target builds the java language bindings to the adevs simulator.
# Point this at your java installation if you want to build the Java bindings.
JAVA_HOME = /usr/local/jdk1.8.0_45
java_adevs: CFLAGS += -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux 

#
# 2) adevs
#
# This builds the vanilla adevs library against which to link your c++ 
# simulations.
#

#
# 3) adevs_qemu
# 
# This builds the vanilla library and the extensions for using qemu
# to insert a simulated computer into your model.
# 

# These are generic options for the GNU C++ compiler.

# Use OpenMP? 
OMP = -fopenmp
# Compiler flags
CFLAGS += ${OMP} -fPIC -Wall -I../include -I./ -I./qemu
# Best bet for GNU compiler
CXX = g++
CC = gcc
OPTFLAG = -g

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

QEMU_OBJS = \
	qemu/computer.o \
	qemu/device_model.o \
	qemu/nic.o \
	qemu/serial_port.o \
	qemu/ucsim_serial_port.o \
	qemu/qqq_rpc.o \
	qemu/ucsim_rpc.o

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
	javac adevs_jni/adevs/*.java
	cd adevs_jni ; jar cfv ../adevs.jar adevs/*.class

# This builds a static library with the qemu extensions
adevs_qemu: ${ADEVS_OBJS} ${QEMU_OBJS}
	ar -r libadevs.a ${ADEVS_OBJS} ${QEMU_OBJS}
	${CXX} -shared -Wl,-soname,libadevs.so.3 -o libadevs.so.3.0 ${ADEVS_OBJS} ${QEMU_OBJS}

install:
	mkdir -p "${DESTDIR}"
	install -m 755 -p libadevs.so.3.0 "${DESTDIR}"
	ln -f -s libadevs.so.3.0 "${DESTDIR}/libadevs.so.3"
	ln -f -s libadevs.so.3.0 "${DESTDIR}/libadevs.so"

clean:
	rm -f *.o core *.a *.so qemu/*.o adevs_jni/*.o adevs_jni/adevs/*.class *.jar rm libadevs.so.* 

#
# Makefile for the GNU g++ compiler.  Use make to build the adevs library with
# this compiler.
#
# Prefix is appended to the include search paths
PREFIX = ..
# Compiler flags
CFLAGS = -pthread -pedantic -Wall -I${PREFIX}/include
# Best bet for GNU compiler
 CC = g++
 OPTFLAG = -O3

#
# SHOULD NOT NEED TO EDIT BELOW THIS LINE
#

OBJS = rv.o mtrand.o poly.o time.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} -c $<

adevs: ${OBJS}
	ar -r libadevs.a ${OBJS}

clean:
	rm -f *.o core *.a

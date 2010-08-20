CFLAGS = -Wall -fopenmp
OPTFLAG = -O3
CC = g++
INCLUDE = -I../../../include 
LIB = -L../../../src -ladevs
OBJS = main.o Sq.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

sim: ${OBJS}
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} ${LIB}

clean:
	rm -f *.o core a.out

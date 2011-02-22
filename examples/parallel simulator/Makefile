CFLAGS = -Wall -fopenmp
OPTFLAG = -g
CC = g++
INCLUDE = -I../../include 
LIB = -L../../src -ladevs
OBJS = main.o 

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

sim: ${OBJS} Genr.h Transcribe.h
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} ${LIB}

clean:
	rm -f *.o core a.out

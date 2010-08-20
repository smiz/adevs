CFLAGS = -Wall 
OPTFLAG = -O3
CC = g++
INCLUDE = -I../../../include 
OBJS = main_bottom_up.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

sim: ${OBJS}
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} 

clean:
	rm -f *.o core a.out

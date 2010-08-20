CFLAGS = -Wall 
OPTFLAG = -O3
CC = g++
LIBS = -lSDL
INCLUDE = -I../../../include 
OBJS = Display.o  main.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

sim: ${OBJS}
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} ${LIBS} 

clean:
	rm -f *.o core a.out

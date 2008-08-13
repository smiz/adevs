CFLAGS = -Wall -g -fopenmp
CC = g++

# Adjust these as needed to find the X11 and adevs libs and headers
ADEVS = ../..
INCLUDE = -I${ADEVS}/include -I../fifo_q_and_server
LIBS = -L${ADEVS}/src -ladevs
##
# Should not need to edit below this line
##

OBJS = Factory.o main.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${INCLUDE} -c $<

all: objs
	${CC} ${CFLAGS} ${OBJS} ${LIBS}

objs: ${OBJS}

clean:
	rm -f *.o core a.out

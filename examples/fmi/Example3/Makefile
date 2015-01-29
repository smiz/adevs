CFLAGS = -Wall -g -fopenmp
CC = g++

# Adjust these as needed to find the X11 and adevs libs and headers
ADEVS = ../..
INCLUDE = -I${ADEVS}/include 
##
# Should not need to edit below this line
##

OBJS = CherryBomb.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${INCLUDE} -c $<

all: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} 

clean:
	rm -f *.o core a.out

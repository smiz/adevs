CC = g++
CFLAGS = -Wall -fopenmp 
PREFIX = ../..
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit anything below this line
##

all: 
	${CC} ${INCLUDE} ${CFLAGS} -o qss qss.cpp
	${CC} ${INCLUDE} ${CFLAGS} -o pc proportional_control.cpp

clean:
	rm -f *.o; rm -f core; rm -f a.out; rm -f pc ; rm -f qss
 


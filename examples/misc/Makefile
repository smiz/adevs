CC = g++
CFLAGS = -O3 -Wall -fopenmp 
PREFIX = ../..
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit anything below this line
##

inv:
	${CC} ${INCLUDE} ${CFLAGS} -o inverter inverter.cpp

all: inverter
	${CC} ${INCLUDE} ${CFLAGS} -o qss qss.cpp
	${CC} ${INCLUDE} ${CFLAGS} -o pc proportional_control.cpp

mda:

clean:
	rm -f *.o; rm -f core; rm -f a.out; rm -f pc ; rm -f qss
 


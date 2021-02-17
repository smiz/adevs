CC = g++
CFLAGS = -O3 -Wall ${INCLUDE}
PREFIX = ../..
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit anything below this line
##

all: inv
	${CC} ${INCLUDE} ${CFLAGS} -o qss qss.cpp
	${CC} ${INCLUDE} ${CFLAGS} -o pc proportional_control.cpp

inv:
	${CC} ${INCLUDE} ${CFLAGS} -o inverter inverter.cpp

clean:
	rm -f *.o; rm -f core; rm -f a.out; rm -f pc ; rm -f qss
	rm -f inverter


ADEVS_PREFIX = ${HOME}/Code/adevs-code
CFLAGS = -std=c++11 -Wall -g -fopenmp
CC = g++
INCLUDE = -I${ADEVS_PREFIX}/include 
LIBS = -L${HOME}/Code/adevs-code/src -ladevs -lrt 

saved:
	${CC} ${CFLAGS} ${INCLUDE} boot_from_saved.cpp ${LIBS} 

ser: 
	${CC} ${CFLAGS} ${INCLUDE} serial.cpp ${LIBS} 

pp: 
	${CC} ${CFLAGS} ${INCLUDE} ping_pong.cpp ${LIBS}


clean: 
	rm -f ${OBJS} a.out 

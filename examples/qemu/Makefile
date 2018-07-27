ADEVS_PREFIX = ${HOME}/Code/adevs-code
CFLAGS = -std=c++11 -Wall -g -fopenmp
CC = g++
INCLUDE = -I${ADEVS_PREFIX}/include 
LIBS = -L${HOME}/Code/adevs-code/src -ladevs -lrt 

ser: 
	${CC} ${CFLAGS} ${INCLUDE} serial.cpp ${LIBS} 

pp: 
	${CC} ${CFLAGS} ${INCLUDE} ping_pong.cpp ${LIBS}


clean: 
	rm -f ${OBJS} a.out pc_serial_*

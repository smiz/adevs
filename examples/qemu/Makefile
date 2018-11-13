ADEVS_PREFIX = ../..
CFLAGS = -std=c++11 -Wall -g -fopenmp
CC = g++
INCLUDE = -I${ADEVS_PREFIX}/include 
LIBS = -L${ADEVS_PREFIX}/src -ladevs -lrt 

gp:
	${CC} ${CFLAGS} ${INCLUDE} gpu.cpp ${LIBS} 

ser: 
	${CC} ${CFLAGS} ${INCLUDE} serial.cpp ${LIBS} 

pp: 
	${CC} ${CFLAGS} ${INCLUDE} ping_pong.cpp ${LIBS}


clean: 
	rm -f ${OBJS} a.out pc_serial_*

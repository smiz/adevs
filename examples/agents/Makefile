CC = g++
CFLAGS = -O3 -Wall -fopenmp -std=c++11
PREFIX = ../..
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit anything below this line
##

sri:
	${CC} ${INCLUDE} ${CFLAGS} sri.cpp -lgsl -lblas

agent1: 
	${CC} ${INCLUDE} ${CFLAGS} a1.cpp -lgsl -lblas

clean:
	rm -f *.o; rm -f core; rm -f a.out
 


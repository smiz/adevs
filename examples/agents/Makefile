CC = g++
CFLAGS = -O3 -Wall -std=c++11
PREFIX = ../..
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit anything below this line
##

cancer:
	${CC} ${INCLUDE} ${CFLAGS} tumor.cpp 

sri:
	${CC} ${INCLUDE} ${CFLAGS} sri.cpp -lgsl -lblas

agent1: 
	${CC} ${INCLUDE} ${CFLAGS} a1.cpp -lgsl -lblas

clean:
	rm -f *.o; rm -f core; rm -f a.out
 


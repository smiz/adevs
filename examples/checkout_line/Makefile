ADEVS_PREFIX = ../..
CFLAGS = -Wall -g -pedantic -fopenmp
CC = g++
INCLUDE = -I${ADEVS_PREFIX}/include

.SUFFIXES: .cpp
.cpp.o:
	${CC} -c ${CFLAGS} ${INCLUDE} $<

all: Clerk2.o Clerk.o Generator.o Observer.o Decision.o \
MultiClerk.o

mc: all main_mc.o
	${CC} ${CFLAGS} main_mc.o Observer.o Decision.o \
	Clerk.o Generator.o MultiClerk.o 

c1: all main_c1.o
	${CC} ${CFLAGS} main_c1.o Observer.o Clerk.o Generator.o

c2: all main_c2.o
	${CC} ${CFLAGS} main_c2.o Observer.o Clerk2.o Generator.o

clean: 
	rm -f *.o core a.out 

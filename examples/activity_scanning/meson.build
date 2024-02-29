# Compiler flags
CFLAGS = -g -Wall -I${HOME}/Code/adevs-code/include -I./ 
# Best bet for GNU compiler
CC = g++

.SUFFIXES: .cpp .c
.cpp.o:
	${CC} ${CFLAGS} -o $@ -c $<

sim: des.o

test2: sim
	${CC} ${CFLAGS} des.o t2.cpp
	./a.out > test2.out
	diff test2.ok test2.out

test1: sim
	${CC} ${CFLAGS} des.o t1.cpp
	./a.out > test1.out
	diff test1.out test1.ok

test3: sim
	${CC} ${CFLAGS} des.o t3.cpp

clean:
	rm -f des.o a.out *.out

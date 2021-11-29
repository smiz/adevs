CFLAGS = -Wall 
OPTFLAG = -Ofast 
CC = g++

# Adjust these as needed to find the X11 and adevs libs and headers
PREFIX = ../..
LIBS = -lpng
LIBPATH = 
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit below this line
##

OBJS = cell.o save_results.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

all: ${OBJS} main.o ca.o
	${CC} ${CFLAGS} -o des ${OPTFLAG} main.o ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}
	${CC} ${CFLAGS} -o ca ${OPTFLAG} ca.o save_results.o ${LIBS} ${LIBPATH} ${INCLUDE}

clean:
	rm -f *.o core des ca


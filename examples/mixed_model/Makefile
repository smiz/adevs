CFLAGS = -Wall -fopenmp
OPTFLAG = -g
CC = g++

# Adjust these as needed to find the X11 and adevs libs and headers
PREFIX = ../..
LIBS = 
LIBPATH = 
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit below this line
##

OBJS = OvenControl.o Mixer.o LengthMonitor.o Oven.o Cleaner.o main.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

all: ${OBJS}
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

clean:
	rm -f *.o core a.out

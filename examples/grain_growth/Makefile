CFLAGS = -Wall 
OPTFLAG = -Ofast
CC = g++

# Adjust these as needed to find the X11 and adevs libs and headers
PREFIX = ../..
LIBS = -lglut -lGL
LIBPATH = -L/usr/X11R6/lib
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit below this line
##

OBJS = main.o cell.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

gfire: ${OBJS}
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

clean:
	rm -f *.o core a.out


CFLAGS = -Wall -fopenmp
OPTFLAG = -O3
CC = g++4

# Adjust these as needed to find the X11 and adevs libs and headers
PREFIX = ../..
LIBS = -lglut
LIBPATH = -L/usr/X11R6/lib
INCLUDE = -I${PREFIX}/include

##
# Should not need to edit below this line
##

OBJS = main.o fireCell.o Configuration.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

gfire: ${OBJS}
	${CC} ${CFLAGS} ${OPTFLAG} ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

clean:
	rm -f *.o core a.out fire_config_*


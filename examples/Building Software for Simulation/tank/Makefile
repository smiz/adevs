OPTFLAG = -O3 -mtune=native -march=native 
CFLAGS = -Wall 
CC = g++

# Adjust these as needed to find X11, GLUT and GL, and the GNU library
# of utilities.
LIBS = -lSDL -lglut -lccgnu2
LIBPATH = -L/usr/X11R6/lib
INCLUDE = -I../../../include

# These objects are common to all variations on the simulator
OBJS = Display.o \
	   SimControl.o \
	   TankPhysics.o \
	   TankPhysicsEqns.o \
	   InterruptHandler.o \
	   PacketProcessing.o \
	   Computer.o \
	   Tank.o 

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${OPTFLAG} ${INCLUDE} -c $<

objs: ${OBJS}

# Build the interactive simulation used in Chapter 2.
sim: objs main.o
	${CC} ${CFLAGS} ${OPTFLAG} main.o ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

# Build the simulator for the lost power experiment in section 2.4.
exp: objs constInputExp.o
	${CC} ${CFLAGS} ${OPTFLAG} constInputExp.o ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

# Build the simulator for the computer. This is discussed in section 4.4.
comp: objs computerTest.o
	${CC} ${CFLAGS} ${OPTFLAG} computerTest.o ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

# Build the simulator for testing single manuevers 
tank: objs tankTest.o
	${CC} ${CFLAGS} ${OPTFLAG} tankTest.o ${OBJS} ${LIBS} ${LIBPATH} ${INCLUDE}

# Build the InterruptHandler simulation. This code is discussed in section 4.1.9.
ih: InterruptHandler.o interruptExperiment.o
	${CC} ${CFLAGS} ${OPTFLAG} interruptExperiment.o InterruptHandler.o ${LIBS} ${LIBPATH} ${INCLUDE}

clean:
	rm -f *.o core a.out

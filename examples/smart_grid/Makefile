# GNU gcc compiler
CFLAGS = -O3 -fopenmp -Wall -I../../include ${OMINCLUDE} 
CC = g++
OMHOME=${HOME}/Code/openmodelica/trunk/build
OMINCLUDE=-I${OMHOME}/include/omc
OMLIBS=-L${OMHOME}/lib/omc -L../../src -ladevs -llapack /usr/local/lib/libf2c.a

sys2:
	omc +s +simCodeTarget=Adevs twobus.mo Thyme.mo Modelica 
	${CC} ${CFLAGS} main_2bus.cpp twobus.cpp ${OMLIBS} 
	./a.out 

clean:
	rm -f a.out soln *.h *.c *.xml *.mat *.libs *.makefile *.log *.plt *.o
	rm -f twobus.cpp  

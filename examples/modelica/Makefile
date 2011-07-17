# GNU gcc compiler
CFLAGS = -g -fopenmp -Wall -I../../include ${OMINCLUDE}
CC = g++
OMHOME=${HOME}/Code/openmodelica/trunk/build
OMINCLUDE=-I${OMHOME}/include/omc
OMLIBS=-L${OMHOME}/lib/omc -L../../src -ladevs -lSimulationRuntimeC -lomcruntime -llapack -lf2c

hello:
	omc +s +simCodeTarget=Adevs HelloWorld.mo 
	${CC} ${CFLAGS} main_hello.cpp HelloWorld.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_hello.mos

influenza:
	omc +s +simCodeTarget=Adevs Influenza.mo
	${CC} ${CFLAGS} main_influenza.cpp Influenza.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_Influenza.mos

bounce:
	omc +s +simCodeTarget=Adevs Bounce.mo
	${CC} ${CFLAGS} main_bounce.cpp Bounce.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_bounce.mos

vanderpol:
	omc +s +simCodeTarget=Adevs VanDerPol.mo
	${CC} ${CFLAGS} main_vanderpol.cpp VanDerPol.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_van.mos

dcm:
	omc +s +simCodeTarget=Adevs dcmotor.mo Modelica
	${CC} ${CFLAGS} main_dcmotor.cpp dcmotor.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_dcmotor.mos

robot:
	omc +s +simCodeTarget=Adevs Robot.mo 
	${CC} ${CFLAGS} Robot.cpp main_robot.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_robot.mos

tank:
	omc +s +simCodeTarget=Adevs Tank.mo Modelica
	${CC} ${CFLAGS} Tank.cpp main_tank.cpp ${OMLIBS} 
	./a.out 1 1 > soln
	omc sim_tank.mos

pendulum:
	omc +s +simCodeTarget=Adevs Pendulum.mo
	${CC} ${CFLAGS} Pendulum.cpp main_pendulum.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_pendulum.mos

nonlinearmixed:
	omc +s +simCodeTarget=Adevs NonlinearMixed.mo
	${CC} ${CFLAGS} NonlinearMixed.cpp main_nonlinearmixed.cpp ${OMLIBS} 
	./a.out > soln
	omc sim_nonlinearmixed.mos

clean:
	rm -f a.out soln *.xml *.mat *.libs *.makefile *.log *.plt *.o
	rm -f Bounce*.c Bounce*.cpp Bounce*.h Bounce.c Bounce 
	rm -f Influenza*.c Influenza*.cpp Influenza*.h Influenza.c Influenza 
	rm -f HelloWorld*.c HelloWorld*.cpp HelloWorld*.h HelloWorld.c HelloWorld 
	rm -f VanDerPol*.c VanDerPol*.cpp VanDerPol*.h VanDerPol.c VanDerPol 
	rm -f dcmotor*.c dcmotor*.cpp dcmotor*.h dcmotor.c dcmotor 
	rm -f Robot*.c Robot*.cpp Robot*.h Robot.c Robot 
	rm -f Tank*.c Tank*.cpp Tank*.h Tank.c Tank
	rm -f Pendulum*.c Pendulum*.cpp Pendulum*.h Pendulum.c Pendulum
	rm -f NonlinearMixed*.c NonlinearMixed*.cpp NonlinearMixed*.h NonlinearMixed.c NonlinearMixed

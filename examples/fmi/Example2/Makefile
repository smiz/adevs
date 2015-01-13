# GNU gcc compiler
OPENMP = -fopenmp
CFLAGS = $(OPENMP) -O3 -pedantic -Wall -I$(PREFIX)/include
CC = g++
LIBS = -L$(PREFIX)/src -ladevs 
PREFIX=$(HOME)/Code/adevs-code
SUNDIALS=/usr/local/sundials
OMHOME=$(HOME)/Code/openmodelica/trunk/build
OMINCLUDE=-I$(OMHOME)/include/omc
OMLIBS=-L$(OMHOME)/lib/omc /usr/local/lib/libf2c.a -lgc 
CFLAGS += $(OMINCLUDE) -I${SUNDIALS}/include
LIBS += $(OMLIBS) -L${SUNDIALS}/lib -lsundials_kinsol -lsundials_nvecserial

quantized:
	omc +s +simCodeTarget=Adevs QuantizedRobot.mo BaseRobot.mo ControlTrajectory.mo
	omc +s +simCodeTarget=Adevs Control.mo ControlTrajectory.mo
	$(CC) $(CFLAGS) Ethernet.cpp Control.cpp Robot.cpp main.cpp $(LIBS)

sampled:
	omc +s +simCodeTarget=Adevs SampledRobot.mo BaseRobot.mo ControlTrajectory.mo
	omc +s +simCodeTarget=Adevs Control.mo ControlTrajectory.mo
	$(CC) $(CFLAGS) Ethernet.cpp Control.cpp Robot.cpp main.cpp $(LIBS)

tst:
	$(CC) $(CFLAGS) Ethernet.cpp netTest.cpp $(LIBS)


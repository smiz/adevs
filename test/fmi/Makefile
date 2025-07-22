# GNU gcc compiler
CC = g++
CFLAGS = -g -Wall -std=c++17
FMI_HOME = $(HOME)/Code/libfmi/install
CFLAGS += -I$(FMI_HOME)/include -I../../include
LIBS += -L../../builddir/src -L$(FMI_HOME)/lib -ladevs -lfmilib
TEST_EXEC = ./a.out

all: t1 te tb tp tei tc se lok_vol

se:
	omc stairs.mos
	$(CC) $(CFLAGS) main_stairs.cpp $(LIBS)  
	$(TEST_EXEC)

tei:
	omc eventIter.mos
	$(CC) $(CFLAGS) main_eventIter.cpp $(LIBS)  
	$(TEST_EXEC)

te:
	omc EventTests.mos
	$(CC) $(CFLAGS) main_EventTests.cpp $(LIBS)  
	$(TEST_EXEC)

t1:
	omc test1.mos
	$(CC) $(CFLAGS) main_test1.cpp $(LIBS) 
	$(TEST_EXEC)

tb:
	omc bounce.mos
	$(CC) $(CFLAGS) main_bounce.cpp $(LIBS) 
	$(TEST_EXEC)

tp:
	rm -rf pendulum; mkdir pendulum; cd pendulum; cp ../pendulum.mo .; cp ../pendulum.mos .; omc pendulum.mos; unzip -o -qq pendulum.fmu
	${PYTHON} ../../utilities/xml2cpp.py -x pendulum/modelDescription.xml -type double -f pendulum/binaries/linux64/pendulum.so -r file://${PWD}/pendulum/resources
	$(CC) $(CFLAGS) main_pendulum.cpp $(LIBS) 
	$(TEST_EXEC)

tc:
	rm -rf circuit; mkdir circuit; cd circuit; cp ../circuit.mo .; cp ../circuit.mos .; omc circuit.mos; unzip -o -qq Circuit.fmu
	${PYTHON} ../../utilities/xml2cpp.py -x circuit/modelDescription.xml -type double -f circuit/binaries/linux64/Circuit.so -r file://${PWD}/circuit/resources
	$(CC) $(CFLAGS) main_circuit.cpp $(LIBS) 
	$(TEST_EXEC)

lok_vol:
	rm -rf lk; mkdir lk; cd lk; cp ../lk.mo .; cp ../lk.mos .; omc lk.mos; unzip -o -qq lk.fmu
	${PYTHON} ../../utilities/xml2cpp.py -x lk/modelDescription.xml -type double -f lk/binaries/linux64/lk.so -r file://${PWD}/lk/resources
	$(CC) $(CFLAGS) main_lk.cpp $(LIBS) -lsundials_kinsol
	$(TEST_EXEC)

clean:
	rm -f *.o a.out
	rm -rf bounce
	rm -rf EventTests
	rm -rf eventIter
	rm -rf test1
	rm -rf pendulum
	rm -rf circuit
	rm -rf stairs
	rm -rf lk

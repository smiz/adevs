PREFIX=../..
include ../make.common
# GNU gcc compiler
FMI_HOME = $(HOME)/Code/FMI_for_ModelExchange_and_CoSimulation_v2.0
CFLAGS += -I$(FMI_HOME)
LIBS += -ldl

all: t1 te tb tp tei tc

tei:
	rm -rf event_tests; mkdir event_tests; cd event_tests; cp ../eventIter.mo .; cp ../eventIter.mos .; omc eventIter.mos; unzip -o -qq eventIter.fmu
	python ../../util/xml2cpp.py -r event_tests/modelDescription.xml -type double -f event_tests/binaries/linux64/eventIter.so
	$(CC) $(CFLAGS) main_eventIter.cpp $(LIBS)  
	$(TEST_EXEC)


te:
	rm -rf event_tests; mkdir event_tests; cd event_tests; cp ../EventTests.mo .; cp ../EventTests.mos .; omc EventTests.mos; unzip -o -qq EventTests.fmu
	python ../../util/xml2cpp.py -r event_tests/modelDescription.xml -type int -f event_tests/binaries/linux64/EventTests.so
	$(CC) $(CFLAGS) main_EventTests.cpp $(LIBS)  
	$(TEST_EXEC)

t1:
	rm -rf test1; mkdir test1; cd test1; cp ../test1.mo .; cp ../test1.mos .; omc test1.mos; unzip -o -qq test1.fmu
	python ../../util/xml2cpp.py -r test1/modelDescription.xml -type int -f test1/binaries/linux64/test1.so
	$(CC) $(CFLAGS) main_test1.cpp $(LIBS) 
	$(TEST_EXEC)

tb:
	rm -rf bounce; mkdir bounce; cd bounce; cp ../bounce.mo .; cp ../bounce.mos .; omc bounce.mos; unzip -o -qq bounce.fmu
	python ../../util/xml2cpp.py -r bounce/modelDescription.xml -type double -f bounce/binaries/linux64/bounce.so
	$(CC) $(CFLAGS) main_bounce.cpp $(LIBS) 
	$(TEST_EXEC)

tp:
	rm -rf pendulum; mkdir pendulum; cd pendulum; cp ../pendulum.mo .; cp ../pendulum.mos .; omc pendulum.mos; unzip -o -qq pendulum.fmu
	python ../../util/xml2cpp.py -r pendulum/modelDescription.xml -type double -f pendulum/binaries/linux64/pendulum.so
	$(CC) $(CFLAGS) main_pendulum.cpp $(LIBS) 
	$(TEST_EXEC)

tc:
	rm -rf circuit; mkdir circuit; cd circuit; cp ../circuit.mo .; cp ../circuit.mos .; omc circuit.mos; unzip -o -qq Circuit.fmu
	python ../../util/xml2cpp.py -r circuit/modelDescription.xml -type double -f circuit/binaries/linux64/Circuit.so
	$(CC) $(CFLAGS) main_circuit.cpp $(LIBS) 
	$(TEST_EXEC)

clean_all: clean
	rm -rf bounce
	rm -rf event_tests
	rm -rf test1
	rm -rf pendulum
	rm -rf circuit

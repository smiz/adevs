PREFIX=../..
include ../make.common
# GNU gcc compiler
FMI_HOME = $(HOME)/Code/fmi/headers
CFLAGS += -I$(FMI_HOME)
LIBS += -ldl
PYTHON=python3

all: t1 te tb tp tei tc se

se:
	rm -rf stairs; mkdir stairs; cd stairs; cp ../stairs.mo .; cp ../stairs.mos .; omc stairs.mos; unzip -o -qq stairs.fmu
	${PYTHON} ../../util/xml2cpp.py -x stairs/modelDescription.xml -type double -f stairs/binaries/linux64/stairs.so -r file://${PWD}/stairs/resources
	$(CC) $(CFLAGS) main_stairs.cpp $(LIBS)  
	$(TEST_EXEC)

tei:
	rm -rf eventIter; mkdir eventIter; cd eventIter; cp ../eventIter.mo .; cp ../eventIter.mos .; omc eventIter.mos; unzip -o -qq eventIter.fmu
	${PYTHON} ../../util/xml2cpp.py -x eventIter/modelDescription.xml -type double -f eventIter/binaries/linux64/eventIter.so -r file://${PWD}/eventIter/resources
	$(CC) $(CFLAGS) main_eventIter.cpp $(LIBS)  
	$(TEST_EXEC)


te:
	rm -rf EventTests; mkdir EventTests; cd EventTests; cp ../EventTests.mo .; cp ../EventTests.mos .; omc EventTests.mos; unzip -o -qq EventTests.fmu
	${PYTHON} ../../util/xml2cpp.py -x EventTests/modelDescription.xml -type int -f EventTests/binaries/linux64/EventTests.so -r file://${PWD}/EventTests/resources
	$(CC) $(CFLAGS) main_EventTests.cpp $(LIBS)  
	$(TEST_EXEC)

t1:
	rm -rf test1; mkdir test1; cd test1; cp ../test1.mo .; cp ../test1.mos .; omc test1.mos; unzip -o -qq test1.fmu
	${PYTHON} ../../util/xml2cpp.py -x test1/modelDescription.xml -type int -f test1/binaries/linux64/test1.so -r file://${PWD}/test1/resources
	$(CC) $(CFLAGS) main_test1.cpp $(LIBS) 
	$(TEST_EXEC)

tb:
	rm -rf bounce; mkdir bounce; cd bounce; cp ../bounce.mo .; cp ../bounce.mos .; omc bounce.mos; unzip -o -qq bounce.fmu
	${PYTHON} ../../util/xml2cpp.py -x bounce/modelDescription.xml -type double -f bounce/binaries/linux64/bounce.so -r file://${PWD}/bounce/resources
	$(CC) $(CFLAGS) main_bounce.cpp $(LIBS) 
	$(TEST_EXEC)

tp:
	rm -rf pendulum; mkdir pendulum; cd pendulum; cp ../pendulum.mo .; cp ../pendulum.mos .; omc pendulum.mos; unzip -o -qq pendulum.fmu
	${PYTHON} ../../util/xml2cpp.py -x pendulum/modelDescription.xml -type double -f pendulum/binaries/linux64/pendulum.so -r file://${PWD}/pendulum/resources
	$(CC) $(CFLAGS) main_pendulum.cpp $(LIBS) 
	$(TEST_EXEC)

tc:
	rm -rf circuit; mkdir circuit; cd circuit; cp ../circuit.mo .; cp ../circuit.mos .; omc circuit.mos; unzip -o -qq Circuit.fmu
	${PYTHON} ../../util/xml2cpp.py -x circuit/modelDescription.xml -type double -f circuit/binaries/linux64/Circuit.so -r file://${PWD}/circuit/resources
	$(CC) $(CFLAGS) main_circuit.cpp $(LIBS) 
	$(TEST_EXEC)

clean_all: clean
	rm -rf bounce
	rm -rf EventTests
	rm -rf eventIter
	rm -rf test1
	rm -rf pendulum
	rm -rf circuit
	rm -rf stairs

PREFIX=../..
include ../make.common
# GNU gcc compiler
FMI_HOME = $(HOME)/Code/FMI_for_ModelExchange_and_CoSimulation_v2.0
CFLAGS += -I$(FMI_HOME)
check: t1

te:
	rm -rf event_tests; mkdir event_tests; cd event_tests; cp ../EventTests.mo .; cp ../EventTests.mos .; omc EventTests.mos; unzip -o -qq EventTests.fmu
	$(CC) $(CFLAGS) main_EventTests.cpp $(LIBS) event_tests/binaries/linux64/EventTests.so 
	$(TEST_EXEC)

t1:
	rm -rf test1; mkdir test1; cd test1; cp ../test1.mo .; cp ../test1.mos .; omc test1.mos; unzip -o -qq test1.fmu
	$(CC) $(CFLAGS) main_test1.cpp $(LIBS) -ldl
	$(TEST_EXEC)

tb:
	rm -rf bounce; mkdir bounce; cd bounce; cp ../bounce.mo .; cp ../bounce.mos .; omc bounce.mos; unzip -o -qq bounce.fmu
	$(CC) $(CFLAGS) main_bounce.cpp $(LIBS) -ldl
	$(TEST_EXEC)

clean_all: clean
	rm -rf bounce
	rm -rf event_tests
	rm -rf test1

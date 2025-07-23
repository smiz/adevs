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
	omc pendulum.mos
	$(CC) $(CFLAGS) main_pendulum.cpp $(LIBS) 
	$(TEST_EXEC)

tc:
	omc circuit.mos
	$(CC) $(CFLAGS) main_circuit.cpp $(LIBS) 
	$(TEST_EXEC)

lok_vol:
	omc lk.mos
	$(CC) $(CFLAGS) main_lk.cpp $(LIBS) -lsundials_kinsol
	$(TEST_EXEC)

clean:
	rm -f a.out *.fmu

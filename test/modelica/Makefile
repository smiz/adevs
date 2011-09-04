PREFIX=../..
include ../make.common
# GNU gcc compiler
OMHOME=$(HOME)/Code/openmodelica/trunk/build
OMINCLUDE=-I$(OMHOME)/include/omc
OMLIBS=-L$(OMHOME)/lib/omc -llapack /usr/local/lib/libf2c.a
CFLAGS += $(OMINCLUDE)
LIBS += $(OMLIBS)

check: t1 t2

t1:
	omc +s +simCodeTarget=Adevs test1.mo 
	$(CC) $(CFLAGS) test1.cpp main_test1.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) test1.cpp test1.h

t2:
	omc +s +simCodeTarget=Adevs test2.mo 
	$(CC) $(CFLAGS) test2.cpp main_test2.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) test2.cpp test2.h

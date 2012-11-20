PREFIX=../..
include ../make.common
# GNU gcc compiler
SUNDIALS=/usr/local/sundials
OMHOME=$(HOME)/Code/openmodelica/trunk/build
OMINCLUDE=-I$(OMHOME)/include/omc
OMLIBS=-L$(OMHOME)/lib/omc -lf2c 
CFLAGS += $(OMINCLUDE) -I${SUNDIALS}/include
LIBS += $(OMLIBS) -L${SUNDIALS}/lib -lsundials_kinsol -lsundials_nvecserial

check: t0 t1 t2 t3 t4

t0:
	omc +s +simCodeTarget=Adevs builtins.mo 
	$(CC) $(CFLAGS) builtins.cpp main_builtins.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) builtins.cpp builtins.h

t1:
	omc +s +simCodeTarget=Adevs test1.mo 
	$(CC) $(CFLAGS) test1.cpp main_test1.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) test1.cpp test1.h

t2:
	omc +s \
		+indexReductionMethod=dynamicStateSelection \
		+matchingAlgorithm=PFPlusExt \
		+simCodeTarget=Adevs test2.mo 
	$(CC) $(CFLAGS) test2.cpp main_test2.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) test2.cpp test2.h

t3:
	omc +s +simCodeTarget=Adevs test3.mo 
	$(CC) $(CFLAGS) Circuit.cpp main_test3.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Circuit.cpp Circuit.h

t4:
	omc +s +simCodeTarget=Adevs test4.mo 
	$(CC) $(CFLAGS) Circuit.cpp main_test4.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Circuit.cpp Circuit.h


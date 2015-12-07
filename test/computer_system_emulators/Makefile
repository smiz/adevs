PREFIX = ../..
include ../make.common

check: ucsim

ucsim: 
	$(CC) $(CFLAGS) ucsim_test.cpp $(LIBS) -lrt
	$(TEST_EXEC) 


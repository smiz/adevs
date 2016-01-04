PREFIX = ../..
include ../make.common

check: ucsim ucsim_serial

ucsim_serial:
	$(CC) $(CFLAGS) ucsim_serial_test.cpp $(LIBS) -lrt

ucsim: 
	$(CC) $(CFLAGS) ucsim_test.cpp $(LIBS) -lrt
	$(TEST_EXEC) 


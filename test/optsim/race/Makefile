PREFIX=../../..
include ../../make.common

check: 
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test.ok tmp

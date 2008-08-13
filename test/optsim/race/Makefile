PREFIX=../../..
include ../../make.common

check: 
	$(CC) $(CFLAGS) main.cpp 
	$(TEST_EXEC) > tmp
	$(COMPARE) test.ok tmp

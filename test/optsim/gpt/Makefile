PREFIX=../../..
include ../../make.common

check: 
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) < test1.in > tmp
	$(COMPARE) test1.out tmp


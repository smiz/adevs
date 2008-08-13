PREFIX=../..
include ../make.common

check: 
	$(CC) $(CFLAGS) main.cpp
	$(TEST_EXEC) < test1.in > tmp
	$(COMPARE) test1.out tmp


PREFIX = ../..
include ../make.common

check:
	$(CC) $(CFLAGS) test.cpp
	$(TEST_EXEC) > tmp
	$(COMPARE) tmp test.ok


PREFIX=../../..
include ../../make.common

check: t1 t2

t1: 
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) < test1.in > tmp
	$(COMPARE) test1.out tmp

t2: 
	$(CC) $(CFLAGS) main_with_chkpt.cpp $(LIBS)
	$(TEST_EXEC) < test1.in > tmp
	$(COMPARE) test1.out tmp


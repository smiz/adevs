PREFIX = ../../..
include ../../make.common

check: t1 t3 t5 t7 t1chkpt t3chkpt t5chkpt t7chkpt

t1:
	$(CC) $(CFLAGS) test1.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test1.ok tmp

t1chkpt:
	$(CC) $(CFLAGS) test1_with_chkpt.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test1.ok tmp

t2:
	$(CC) $(CFLAGS) test2.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test2.ok tmp

t3:
	$(CC) $(CFLAGS) test3.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test3.ok tmp

t3chkpt:
	$(CC) $(CFLAGS) test3_with_chkpt.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test3.ok tmp

t4:
	$(CC) $(CFLAGS) test4.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test4.ok tmp

t5:
	$(CC) $(CFLAGS) test5.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test5.ok tmp

t5chkpt:
	$(CC) $(CFLAGS) test5_with_chkpt.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test5.ok tmp

t6:
	$(CC) $(CFLAGS) test6.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test6.ok tmp

t7:
	$(CC) $(CFLAGS) test7.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test7.ok tmp

t7chkpt:
	$(CC) $(CFLAGS) test7_with_chkpt.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test7.ok tmp

t8:
	$(CC) $(CFLAGS) test8.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test8.ok tmp

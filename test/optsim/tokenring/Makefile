PREFIX=../../..
include ../../make.common

check: t1 t2 t3 t4 t5 t6 t7

t1:
	$(CC) $(CFLAGS) case1.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test1.ok tmp

t2: 
	$(CC) $(CFLAGS) case2.cpp $(LIBS) 
	$(TEST_EXEC) > tmp
	$(COMPARE) test2.ok tmp

t3: 
	$(CC) $(CFLAGS) case3.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test3.ok tmp

t4: 
	$(CC) $(CFLAGS) case4.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test4.ok tmp

t5: 
	$(CC) $(CFLAGS) case5.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test5.ok tmp

t6: 
	$(CC) $(CFLAGS) case6.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test6.ok tmp

t7: 
	$(CC) $(CFLAGS) case7.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(COMPARE) test7.ok tmp

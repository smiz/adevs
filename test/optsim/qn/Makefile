PREFIX=../../..
include ../../make.common

check: check1 check2 check3 check4

check2:
	$(CC) $(CFLAGS) test2.cpp $(LIBS)
	$(TEST_EXEC) 
	$(MOVE) c.dat c1.dat
	$(MOVE) g.dat g1.dat
	$(TEST_EXEC) -par
	$(DIFF) g.dat g1.dat
	$(DIFF) c.dat c1.dat

check3:
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) 20 20 1000 -recycle
	$(MOVE) c.dat c1.dat
	$(MOVE) g.dat g1.dat
	$(TEST_EXEC) 20 20 1000 -par -recycle
	$(DIFF) g.dat g1.dat
	$(DIFF) c.dat c1.dat

check1:
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) 20 20 1000
	$(MOVE) c.dat c1.dat
	$(MOVE) g.dat g1.dat
	$(TEST_EXEC) 20 20 1000 -par
	$(DIFF) g.dat g1.dat
	$(DIFF) c.dat c1.dat

check4:
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) 1 1 1000 -recycle
	$(MOVE) c.dat c1.dat
	$(MOVE) g.dat g1.dat
	$(TEST_EXEC) 1 1 1000 -par -recycle
	$(DIFF) g.dat g1.dat
	$(DIFF) c.dat c1.dat

clean_all: clean
	$(DEL) *.dat

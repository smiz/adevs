PREFIX=../../..
include ../../make.common

check: check1 check2

check2:
	$(CC) $(CFLAGS) test2.cpp $(LIBS)
	$(TEST_EXEC) 
	mv c.dat c1.dat
	mv g.dat g1.dat
	$(TEST_EXEC) p
	diff g.dat g1.dat
	diff c.dat c1.dat

check1:
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) 20 20 1000
	mv c.dat c1.dat
	mv g.dat g1.dat
	$(TEST_EXEC) 20 20 1000 p
	diff g.dat g1.dat
	diff c.dat c1.dat

clean_all: clean
	$(DEL) *.dat

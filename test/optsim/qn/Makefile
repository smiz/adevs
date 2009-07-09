PREFIX=../../..
include ../../make.common

check:
	$(CC) $(CFLAGS) main.cpp $(LIBS)
	$(TEST_EXEC) 20 20 1000
	mv c.dat c1.dat
	mv g.dat g1.dat
	$(TEST_EXEC) 20 20 1000 p
	diff g.dat g1.dat
	diff c.dat c1.dat

clean_all: clean
	$(DEL) *.dat

PREFIX=../../..
include ../../make.common

check:
	$(CC) $(CFLAGS) -I/usr/local/tau/include fireCell.cpp Configuration.cpp main.cpp $(LIBS)
#	$(TEST_EXEC) --config test.dat 
	$(TEST_EXEC) --config test.dat -p
	diff soln_0 soln_0_p
	diff soln_1 soln_1_p
	diff soln_2 soln_2_p
	diff soln_3 soln_3_p
	diff soln_4 soln_4_p
	diff soln_5 soln_5_p
	diff soln_6 soln_6_p
	diff soln_7 soln_7_p
	diff soln_8 soln_8_p
	diff soln_9 soln_9_p

clean_all: clean
	$(DEL) soln*

PREFIX = ../..
include ../make.common

check: bnew dae dae2

dae2: 
	$(CC) $(CFLAGS) dae_test2.cpp
	$(TEST_EXEC) 1> tmp 2> tmp

dae:
	$(CC) $(CFLAGS) dae_test1.cpp
	$(TEST_EXEC) 1> tmp 2> tmp

bnew:
	$(CC) $(CFLAGS) ball1d_new.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp


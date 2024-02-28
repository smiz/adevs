PREFIX = ../..
include ../make.common

check: bnew dae dae2 con compare

con:
	$(CC) $(CFLAGS) confluent_test.cpp $(ODE_LIBS)
	$(TEST_EXEC)

dae2: 
	$(CC) $(CFLAGS) dae_test2.cpp $(ODE_LIBS)
	$(TEST_EXEC) 1> tmp1 2> tmp2

dae:
	$(CC) $(CFLAGS) dae_test1.cpp $(ODE_LIBS)
	$(TEST_EXEC) 1> tmp1 2> tmp2

bnew:
	$(CC) $(CFLAGS) ball1d_new.cpp check_ball1d_solution.cpp $(ODE_LIBS)
	$(TEST_EXEC) > tmp

compare:
	$(CC) $(CFLAGS) ode.cpp $(ODE_LIBS) 
	$(TEST_EXEC) > tmp

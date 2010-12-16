PREFIX = ../..
include ../make.common

check: bnew rk4_test rk45_test rk45_improved_test dae dae2

dae2: 
	$(CC) $(CFLAGS) dae_test2.cpp
	$(TEST_EXEC) 1> tmp 2> tmp

dae:
	$(CC) $(CFLAGS) dae_test1.cpp
	$(TEST_EXEC) 1> tmp 2> tmp

bnew:
	$(CC) $(CFLAGS) ball1d_new.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp

rk4_test:
	$(CC) $(CFLAGS) rk4_test1.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp
	$(DEL) state.dat

rk45_test:
	$(CC) $(CFLAGS) rk45_test1.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp
	$(DEL) state.dat

rk45_improved_test:
	$(CC) $(CFLAGS) ball1d_improved.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp
	$(DEL) state.dat

ball2dv2_test:
	$(CC) $(CFLAGS) ball2dv2.cpp

ball2dv2_rk45_test:
	$(CC) $(CFLAGS) ball2dv2_rk45.cpp

ball2d_test:
	$(CC) $(CFLAGS) ball2d.cpp

ball1d_rk45_test:
	$(CC) $(CFLAGS) ball1d_rk45.cpp

ball1d_rk45_improved_test:
	$(CC) $(CFLAGS) ball1d_rk45_improved.cpp


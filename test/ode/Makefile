PREFIX = ../..
include ../make.common

check: rk4_test rk45_test rk45_improved_test

rk4_test:
	$(CC) $(CFLAGS) rk4_test1.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp

rk45_test:
	$(CC) $(CFLAGS) rk45_test1.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp

rk45_improved_test:
	$(CC) $(CFLAGS) ball1d_improved.cpp check_ball1d_solution.cpp
	$(TEST_EXEC) > tmp

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

dist_clean: clean


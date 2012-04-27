PREFIX=..
include make.common

# Everything else should work fine
check: check_cpp check_par check_java check_modelica

# Check cpp code only
check_cpp: bag_test obj_pool sched atomic double_fcmp gcd_test gpt_test \
tokenring_test dyn_devs_test zero_time_test ode_test listener_test \
wrapper_test alt_time

# Check OpenMP code
check_par: optsim_test  

# Check the java library
check_java: java_test

# Check the modelica runtime system and compiler
check_modelica:
	cd modelica $(CMD_SEP) $(MAKE) check

java_test:
	cd java $(CMD_SEP) $(MAKE) check

optsim_test:
	cd optsim $(CMD_SEP) $(MAKE) check

wrapper_test:
	cd wrapper $(CMD_SEP) $(MAKE) check

listener_test:
	cd listener $(CMD_SEP) $(MAKE) check

ode_test:
	cd ode $(CMD_SEP) $(MAKE) check

zero_time_test:
	cd zero_time $(CMD_SEP) $(MAKE) check

dyn_devs_test:
	cd dyn_devs $(CMD_SEP) $(MAKE) check

gpt_test:
	cd gpt $(CMD_SEP) $(MAKE) check

gcd_test:
	cd gcd $(CMD_SEP) $(MAKE) check

tokenring_test:
	cd tokenring $(CMD_SEP) $(MAKE) check

bag_test:
	$(CC) $(CFLAGS) bagtst.cpp
	$(TEST_EXEC)

alt_time:
	$(CC) $(CFLAGS) alt_time_tests.cpp 
	$(TEST_EXEC)

mt_test:
	$(CC) $(CFLAGS) mttst.cpp $(LIBS)
	$(TEST_EXEC)

obj_pool:
	$(CC) $(CFLAGS) object_pool_test.cpp 
	$(TEST_EXEC)

sched:
	$(CC) $(CFLAGS) sched_test.cpp 
	$(TEST_EXEC)

atomic:
	$(CC) $(CFLAGS) atomic_test.cpp 
	$(TEST_EXEC)

double_fcmp:
	$(CC) $(CFLAGS) double_fcmp_test.cpp 
	$(TEST_EXEC)

clean_all:
	cd gcd $(CMD_SEP) $(MAKE) clean
	cd gpt $(CMD_SEP) $(MAKE) clean
	cd tokenring $(CMD_SEP) $(MAKE) clean
	cd listener $(CMD_SEP) $(MAKE) clean
	cd dyn_devs $(CMD_SEP) $(MAKE) clean
	cd zero_time $(CMD_SEP) $(MAKE) clean
	cd ode $(CMD_SEP) $(MAKE) clean
	cd wrapper $(CMD_SEP) $(MAKE) clean
	cd optsim $(CMD_SEP) $(MAKE) clean_all
	cd java $(CMD_SEP) $(MAKE) clean
	cd modelica $(CMD_SEP) $(MAKE) clean
	$(MAKE) clean


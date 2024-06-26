include make.common

# Everything else should work fine
check: check_cpp check_java check_fmi

# Check cpp code only
check_cpp: rvtest bag_test obj_pool sched atomic double_fcmp gcd_test gpt_test \
tokenring_test dyn_devs_test zero_time_test ode_test listener_test \
wrapper_test alt_time mealy sd_time

# Check the java library
check_java: java_test

# Check the fmi support
check_fmi: 
	cd fmi $(CMD_SEP) $(MAKE) all

java_test:
	cd java $(CMD_SEP) $(MAKE) check

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

rvtest:
	$(CC) $(CFLAGS) rv_test.cpp $(LIBS)
	$(TEST_EXEC)

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

sd_time:
	$(CC) $(CFLAGS) sd_time_test.cpp 
	$(TEST_EXEC)
	$(CC) $(CFLAGS) sd_time_test_2.cpp
	$(TEST_EXEC) > tmp
	$(DIFF) tmp sd_time_test_2.ok

atomic:
	$(CC) $(CFLAGS) atomic_test.cpp 
	$(TEST_EXEC)

double_fcmp:
	$(CC) $(CFLAGS) double_fcmp_test.cpp 
	$(TEST_EXEC)

mealy:
	$(CC) $(CFLAGS) test_mealy.cpp
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
	cd java $(CMD_SEP) $(MAKE) clean
	cd fmi $(CMD_SEP) $(MAKE) clean_all
	$(MAKE) clean


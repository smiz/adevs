# Add tests in their own subdirectories
subdir('dyn_devs')
# subdir('fmi')
# subdir('gcd')
subdir('gpt')
subdir('listener')
# subdir('ode')
subdir('poly')
subdir('tokenring')
# subdir('wrapper')
subdir('zero_time')

# Tests in this directory

test_alt_time = executable('alt_time', 'alt_time_tests.cpp', include_directories: adevs)
test('alt_time', test_alt_time)

test_atomic = executable('atomic', 'atomic_test.cpp', include_directories: adevs)
test('atomic', test_atomic)

test_double_fcmp = executable('double_fcmp', 'double_fcmp_test.cpp', include_directories: adevs)
test('double_fcmp', test_double_fcmp)

test_sched = executable('sched', 'sched_test.cpp', include_directories: adevs)
test('sched', test_sched)

test_schedule2 = executable('schedule2', 'sched_test2.cpp', include_directories: adevs)
test('schedule2', test_schedule2)

benchmark_schedule1 = executable('benchmark_schedule1', 'sched_benchmark1.cpp', include_directories: adevs)

benchmark_schedule2 = executable('benchmark_schedule2', 'sched_benchmark2.cpp', include_directories: adevs)

test_sd_time = executable('sd_time', 'sd_time_test.cpp', include_directories: adevs)
test('sd_time', test_sd_time)

test_sd_time_2 = executable('sd_time_2', 'sd_time_test_2.cpp', include_directories: adevs)
test_sd_time_2_ok = fs.copyfile('sd_time_test_2.ok')
test(
    'sd_time_2',
    compare,
    args: [test_sd_time_2.full_path(), test_sd_time_2_ok.full_path()],
)

test_mealy = executable('mealy', 'test_mealy.cpp', include_directories: adevs)
test('mealy', test_mealy)

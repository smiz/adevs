# Copy the compare file to the build
#fs.copyfile('compare.sh')
compare = find_program('compare.sh')

# Add tests in their own subdirectories
#subdir('dyn_devs')
#subdir('fmi')
#subdir('gcd')
#subdir('gpt')
#subdir('java')
#subdir('listener')
#subdir('ode')
#subdir('poly')
#subdir('tokenring')
#subdir('wrapper')
#subdir('zero_time')


# Tests in this directory

test_alt_time = executable('alt_time', 'alt_time_tests.cpp', include_directories: adevs)
test('alt_time', test_alt_time)

test_atomic = executable('atomic', 'atomic_test.cpp', include_directories: adevs)
test('atomic', test_atomic)

test_bag = executable('bag_test', 'bag_test.cpp', include_directories: adevs)
test('bag', test_bag)

test_double_fcmp = executable('double_fcmp', 'double_fcmp_test.cpp', include_directories: adevs)
test('double_fcmp', test_double_fcmp)

# TODO: Linking/compiling issue
# test_mt_test = executable('mt_test', 'mt_test.cpp', include_directories: adevs)
# test('mt_test', test_mt_test)

test_obj_pool = executable('obj_pool', 'object_pool_test.cpp', include_directories: adevs)
test('obj_pool', test_obj_pool)

test_rvtest = executable('rvtest', 'rv_test.cpp', include_directories: adevs)
test('rvtest', test_rvtest)

test_sched = executable('sched', 'sched_test.cpp', include_directories: adevs)
test('sched', test_sched)

test_sd_time = executable('sd_time', 'sd_time_test.cpp', include_directories: adevs)
test('sd_time', test_sd_time)

test_sd_time_2 = executable('sd_time_2', 'sd_time_test_2.cpp', include_directories: adevs)
test('sd_time_2', test_sd_time_2)
# TODO: Compare to sd_time_test_2.ok

test_mealy = executable('mealy', 'test_mealy.cpp', include_directories: adevs)
test('mealy', test_mealy)

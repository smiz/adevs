test_zero_time = executable('zero_time', 'test.cpp', include_directories: adevs)
test_zero_time_ok = fs.copyfile('test.ok')
test('zero_time', run_and_compare, args: [test_zero_time, test_zero_time_ok])

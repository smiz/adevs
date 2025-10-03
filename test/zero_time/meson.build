test_zero_time = executable('zero_time', 'test.cpp', include_directories: adevs, link_with: adevs_lib)
test_zero_time_ok = fs.copyfile('test.ok')
test('zero_time', run_and_compare, args: [test_zero_time, test_zero_time_ok])

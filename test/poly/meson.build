test_poly = executable('poly_test', 'poly_test.cpp', include_directories: adevs)
test_poly_soln = fs.copyfile('solution')
test('poly', compare, args: [test_poly.full_path(), test_poly_soln.full_path()])

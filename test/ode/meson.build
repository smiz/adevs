test_con = executable('con', 'confluent_test.cpp', include_directories: adevs, link_with: adevs_lib)
test('ode-con', test_con)

test_bnew = executable('bnew', 'ball1d_new.cpp', 'check_ball1d_solution.cpp', include_directories: adevs, link_with: adevs_lib)
test('ode-bnew', test_bnew)

test_cvode = executable('cvode', 'cvode.cpp', 'check_ball1d_solution.cpp', include_directories: adevs, link_with: adevs_lib, link_args: '-lsundials_cvode')
test('cvode', test_cvode)

test_compare = executable('compare', 'ode.cpp', include_directories: adevs, link_with: adevs_lib, link_args: '-lsundials_kinsol')
test('ode-compare', test_compare)

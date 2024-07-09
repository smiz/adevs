
test_con = executable('con', 'confluent_test.cpp', include_directories: adevs)
test('ode-con', test_con)

test_dae2  = executable('dae2', 'dae_test2.cpp', include_directories: adevs)
test('ode-dae2', test_dae2)

test_dae = executable('dae', 'dae_test1.cpp', include_directories: adevs)
test('ode-dae', test_dae)

test_bnew = executable('bnew', 'ball1d_new.cpp', 'check_ball1d_solution.cpp', include_directories: adevs)
test('ode-bnew', test_bnew)

# TODO: Linker/compiler issue
# test_compare = executable('compare', 'ode.cpp', include_directories: adevs)
# test('ode-compare', test_compare)

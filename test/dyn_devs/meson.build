test_dyn_devs_add = executable('add_test', ['SimpleAtomic.cpp', 'add_test.cpp'], include_directories: adevs, link_with: adevs_lib)
test('dyn_devs_add', test_dyn_devs_add)

test_dyn_devs_remove = executable('remove_test', ['SimpleAtomic.cpp', 'remove_test.cpp'], include_directories: adevs, link_with: adevs_lib)
test('dyn_devs_remove', test_dyn_devs_remove)

test_dyn_devs_moved = executable('moved_test', ['SimpleAtomic.cpp', 'moved_test.cpp'], include_directories: adevs, link_with: adevs_lib)
test('dyn_devs_moved', test_dyn_devs_moved)

test_dyn_coupled = executable('coupled_test', ['coupled_test.cpp'], include_directories: adevs, link_with: adevs_lib)
test('dyn_devs_coupled', test_dyn_coupled)
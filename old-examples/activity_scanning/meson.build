activity_test1 = executable(
    'test1',
    ['des.cpp', 't1.cpp'],
    include_directories: adevs,
)
activity_test1_ok = fs.copyfile('test1.ok')
test(
    'activity_scanning-test1',
    compare,
    args: [activity_test1.full_path(), activity_test1_ok.full_path()],
)

activity_test2 = executable(
    'test2',
    ['des.cpp', 't2.cpp'],
    include_directories: adevs,
)
activity_test2_ok = fs.copyfile('test2.ok')
test(
    'activity_scanning-test2',
    compare,
    args: [activity_test2.full_path(), activity_test2_ok.full_path()],
)

activity_test3 = executable(
    'test3',
    ['des.cpp', 't3.cpp'],
    include_directories: adevs,
)
activity_test3_ok = fs.copyfile('test3.ok')
test(
    'activity_scanning-test3',
    compare,
    args: [activity_test3.full_path(), activity_test3_ok.full_path()],
)
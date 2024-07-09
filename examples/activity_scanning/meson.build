# TODO: Try and use meson tests instead.
fs.copyfile('test.sh')
fs.copyfile('test1.ok')
fs.copyfile('test2.ok')

test1 = executable(
    'test1',
    ['des.cpp', 't1.cpp'],
    include_directories: adevs,
)

test2 = executable(
    'test2',
    ['des.cpp', 't2.cpp'],
    include_directories: adevs,
)

test3 = executable(
    'test3',
    ['des.cpp', 't3.cpp'],
    include_directories: adevs,
)

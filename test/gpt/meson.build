test_gpt = executable('gpt', 'main.cpp', include_directories: adevs)
test_gpt_in = fs.copyfile('test1.in')
test_gpt_out = fs.copyfile('test1.out')
test('gpt', run_and_compare, args: [test_gpt.full_path(), test_gpt_in.full_path(), test_gpt_out.full_path()])

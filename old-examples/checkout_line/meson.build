checkout_common_cpp = [
    'Clerk2.cpp',
    'Clerk.cpp',
    'Generator.cpp',
    'Observer.cpp',
    'Decision.cpp',
    'MultiClerk.cpp',
]

checkout_common = static_library(
    'checkout_common',
    checkout_common_cpp,
    include_directories: adevs,
    gnu_symbol_visibility: 'hidden',
)

checkout_input = fs.copyfile('input')

checkout_c1 = executable(
    'c1',
    'main_c1.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)
checkout_c1_ok = fs.copyfile('output_c1.ok')
test(
    'checkout_c1',
    run_and_compare_output,
    args: [
        checkout_c1.full_path(),
        checkout_input.full_path(),
        './output_c1',
        checkout_c1_ok.full_path(),
    ],
)

checkout_c2 = executable(
    'c2',
    'main_c2.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)
checkout_c2_ok = fs.copyfile('output_c2.ok')
test(
    'checkout_c2',
    run_and_compare_output,
    args: [
        checkout_c1.full_path(),
        checkout_input.full_path(),
        './output_c2',
        checkout_c1_ok.full_path(),
    ],
)

checkout_mc = executable(
    'mc',
    'main_mc.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)
checkout_mc_ok = fs.copyfile('output_mc.ok')
# test(
#     'checkout_mc',
#     run_and_compare_output,
#     args: [
#         checkout_mc.full_path(),
#         checkout_input.full_path(),
#         './output_mc',
#         checkout_mc_ok.full_path(),
#     ],
# )

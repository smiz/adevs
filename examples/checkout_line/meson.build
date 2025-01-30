checkout_common_cpp = [
    'Clerk2.cpp',
    'Clerk.cpp',
    'Generator.cpp',
    'Observer.cpp',
    #'Decision.cpp',
    #'MultiClerk.cpp',
]

example_checkout_input = fs.copyfile('input')

checkout_common = static_library(
    'checkout_common',
    checkout_common_cpp,
    include_directories: adevs,
    gnu_symbol_visibility: 'hidden',
)

executable(
    'c1',
    'main_c1.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)
example_checkout_c1_ok = fs.copyfile('output_c1.ok')

executable(
    'c2',
    'main_c2.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)
example_checkout_c2_ok = fs.copyfile('output_c2.ok')

# executable(
#     'mc',
#     'main_mc.cpp',
#     include_directories: adevs,
#     link_with: checkout_common,
# )
# example_checkout_mc_ok = fs.copyfile('output_mc.ok')

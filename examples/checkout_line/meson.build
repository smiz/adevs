checkout_common_cpp = [
    'Clerk2.cpp',
    'Clerk.cpp',
    'Generator.cpp',
    'Observer.cpp',
    'Decision.cpp',
    'MultiClerk.cpp'
]

checkout_common = static_library(
    'checkout_common',
    checkout_common_cpp,
    include_directories: adevs,
    gnu_symbol_visibility: 'hidden',
)

executable(
    'mc',
    'main_mc.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)

executable(
    'c1',
    'main_c1.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)

executable(
    'c2',
    'main_c2.cpp',
    include_directories: adevs,
    link_with: checkout_common,
)

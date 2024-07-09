all_cpp = [
    'Clerk2.cpp',
    'Clerk.cpp',
    'Generator.cpp',
    'Observer.cpp',
    'Decision.cpp',
    'MultiClerk.cpp'
]

checkout = static_library(
    'checkout',
    all_cpp,
    include_directories: adevs,
    gnu_symbol_visibility: 'hidden',
)

executable(
    'mc',
    'main_mc.cpp',
    include_directories: adevs,
    link_with: checkout,
)

executable(
    'c1',
    'main_c1.cpp',
    include_directories: adevs,
    link_with: checkout,
)

executable(
    'c2',
    'main_c2.cpp',
    include_directories: adevs,
    link_with: checkout,
)

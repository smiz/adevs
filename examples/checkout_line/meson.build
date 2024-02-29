# CFLAGS = -Wall -g -pedantic

all_cpp = ['Clerk2.cpp', 'Clerk.cpp', 'Generator.cpp', 'Observer.cpp', 'Decision.cpp', 'MultiClerk.cpp']

checkout = static_library(
  'checkout',
  all_cpp,
  gnu_symbol_visibility: 'hidden',
  include_directories: adevs,
)

executable('mc', 'main_mc.cpp', include_directories: adevs, link_with: checkout)
executable('c1', 'main_c1.cpp', include_directories: adevs, link_with: checkout)
executable('c2', 'main_c2.cpp', include_directories: adevs, link_with: checkout)

# -- Without the static library --

# mc_cpp = ['main_mc.cpp', 'Clerk.cpp', 'Generator.cpp', 'Observer.cpp', 'Decision.cpp', 'MultiClerk.cpp']
# executable('mc', mc_cpp, include_directories: adevs)

# c1_cpp = ['main_c1.cpp', 'Clerk.cpp', 'Generator.cpp', 'Observer.cpp']
# executable('c1', c1_cpp, include_directories: adevs)

# c2_cpp = ['main_c2.cpp', 'Clerk2.cpp', 'Generator.cpp', 'Observer.cpp']
# executable('c2', c2_cpp, include_directories: adevs)

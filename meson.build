project(
  'adevs',
  'cpp',
  version: '4.0-dev',
  default_options: [
    'warning_level=3',
    'cpp_std=c++17',
  ],
)

project_description = '''
    Adevs (A Discrete EVent System simulator) is a C++ library for constructing discrete event
    simulations based on the Parallel DEVS and Dynamic DEVS (dynDEVS) formalisms. DEVS has been
    applied to the study of social systems, ecological systems, computer networks and computer
    architecture, military systems at the tactical and theater levels, and in many other areas.
    Recent advances in quantized approximations of continuous systems suggest promising
    computational techniques for high performance scientific computing (e.g. in the field of
    computational fluid dynamics).
'''

adevs = include_directories('include')

# Imports used other places
fs = import('fs')

# Scripts/helpers for tests
compare = find_program('utilities/compare.sh')
run_and_compare = find_program('utilities/run_and_compare.sh')

# Add all the subdirectoriess
#subdir('examples')
subdir('src')
subdir('test')

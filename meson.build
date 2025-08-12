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

# Dependencies for Sundials
# If Sundials is not default/system-wide installed one should set the CMAKE_PREFIX_PATH environmental variable
# before running "meson setup build_dir" 
sundials_dep = dependency('SUNDIALS', method: 'cmake', modules: ['SUNDIALS::core', 'SUNDIALS::cvode', 'SUNDIALS::kinsol'])

# Scripts/helpers for tests
compare = find_program('utilities/compare.sh')
run_and_compare = find_program('utilities/run_and_compare.sh')
run_and_compare_output = find_program('utilities/run_and_compare_output.sh')

# Add all the subdirectoriess
#subdir('examples')
subdir('src')
subdir('test')

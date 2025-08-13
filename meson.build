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

dep_check = dependency('SUNDIALS', method: 'cmake', modules: ['SUNDIALS::cvode'], required: true)
message('Found SUNDIALS version: ' + dep_check.version())

if dep_check.version().version_compare('>=7')
  sundials_dep = dependency('SUNDIALS', method: 'cmake', modules: ['SUNDIALS::core', 'SUNDIALS::cvode', 'SUNDIALS::kinsol'])
  message('SUNDIALS >= 7: using modules: core, cvode, kinsol')
else
  sundials_dep = dependency('SUNDIALS', method: 'cmake', modules: ['SUNDIALS::cvode', 'SUNDIALS::kinsol'])
  message('SUNDIALS < 7; using modules: cvode, kinsol')
endif

# Scripts/helpers for tests
compare = find_program('utilities/compare.sh')
run_and_compare = find_program('utilities/run_and_compare.sh')
run_and_compare_output = find_program('utilities/run_and_compare_output.sh')

# Add all the subdirectoriess
#subdir('examples')
subdir('src')
subdir('test')


# Additional targets for clang-tidy and Doxygen

# clang-tidy: usage "meson compile -C build tidy"
# Output is to stdout. To redirect ot file: meson compile -C build tidy > /path/clang-tidy-output.txt
# If no path is given the output file is in the directory from which the command is given.
clang_tidy = find_program('clang-tidy', required: false)

# Try both common helper names
run_clang_tidy = find_program('run-clang-tidy', required: false)
if not run_clang_tidy.found()
  run_clang_tidy = find_program('run-clang-tidy.py', required: false)
endif

if clang_tidy.found()
  # Choose the command using a normal if/else (Meson has no ternary)
  tidy_cmd = []
  if run_clang_tidy.found()
    message('Running run-clang-tidy')
    tidy_cmd = [run_clang_tidy, '-p', meson.current_build_dir()]
  else
    message('Running clang-tidy')
    tidy_cmd = [clang_tidy, '-p', meson.current_build_dir(), '--quiet'] + srcs
  endif

  run_target(
    'tidy',                 # run with: meson compile -C build tidy
    command : tidy_cmd
  )
endif

# doxygen: usage meson compile -C build docs
# The output is in the build/docs/html directory
doxygen = find_program('doxygen', required: false)

if doxygen.found()
  doxy_outdir = join_paths(meson.current_build_dir(), 'docs')

  doxyfile = configure_file(
    input  : 'Doxyfile.in',
    output : 'Doxyfile',
    configuration : {
      'PROJECT_NAME'   : meson.project_name(),
      'PROJECT_NUMBER' : meson.project_version(),
      'DOXY_SRC'       : meson.project_source_root(),
      'DOXY_OUT'       : doxy_outdir,
    }
  )

  # Build docs with: meson compile -C build docs
  run_target(
    'docs',
    command : [doxygen, doxyfile]
  )
endif

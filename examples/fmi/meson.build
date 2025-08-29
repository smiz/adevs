
fmi_inc    = include_directories(get_option('fmi_inc'))
fmi_libdir = get_option('fmi_libdir')

# If, at this point, fmi_inc and/or fmi_libdir are empty, that means that nobody
# used meson_options.txt, meson_native.ini or -D command line options to provide
# the location of fmi.h and/or libfmilib_shared.so. As a last resort attempt we
# fall back to Meson's "just compile and link" strategy finding headers and libraries.
# If that also fails, we bail out.
 
if get_option('fmi_inc')  == ''
message('fmi_inc == ')
cc = meson.get_compiler('cpp')
fmi_inc = cc.has_header('fmi.h', required: true)
endif

if get_option('fmi_libdir') == ''
message('fmi_libdir ==  ')
cc = meson.get_compiler('cpp')
fmi_libdir = cc.find_library('fmilib_shared', required: true)
endif

fmi_dep = declare_dependency(
  include_directories: fmi_inc,
  link_args: ['-L' + fmi_libdir, '-lfmilib_shared', '-Wl,-rpath,' + fmi_libdir]
)
executable(
    'circuit',
    'circuit.cpp',
    include_directories: adevs, link_with: adevs_lib, dependencies: [fmi_dep,sundials_dep],
)

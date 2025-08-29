# Get dependencies for all of the examples at a single time.

cc = meson.get_compiler('cpp')
d_png = cc.find_library('png', required: false, disabler: true)
d_glut = cc.find_library('glut', required: false, disabler: true)

d_gl = dependency('GL', required: false, disabler: true)
d_gsl = dependency('gsl', required: false, disabler: true)
d_blas = dependency('blas', required: false, disabler: true)
d_sdl = dependency('SDL', required: false, disabler: true)
d_lapack = dependency('lapack', required: false, disabler: true)

# Subdirectories for each example

subdir('fmi')
subdir('game_of_life')
subdir('tutorial')

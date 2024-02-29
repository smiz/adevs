gsl = dependency('gsl')
blas = dependency('blas')

executable('cancer', 'tumor.cpp', include_directories: adevs)
executable('sri', 'sri.cpp', dependencies: [gsl, blas], include_directories: adevs)
executable('agent1', 'a1.cpp', dependencies: [gsl, blas], include_directories: adevs)

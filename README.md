# A Discrete EVent system Simulator (adevs) 

This is an implementation of the DEVS modeling and simulation
formalism. For a quick start run

make

This will build a static library libadevs.a and the
HTML documentation for the API. The documentation appears in
the html directory and you can get started with it by opening
the file html/index.html with your browser.

If you just want the library, then use the command

make lib

If you just want the documentation, then use the command

make docs

To cleanup the library and docs, then use the command

make clean

## Testing

If you want to run the tests you will need to install meson.
You will also need the [SUNDIALS](https://computing.llnl.gov/projects/sundials)
and [fmi-library](https://github.com/modelon-community/fmi-library)development libraries.
To build and run the tests use

meson setup builddir\
meson compile -C builddir\
meson test -C builddir




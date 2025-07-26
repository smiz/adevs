#ifndef HelloWorld_h_
#define HelloWorld_h_
#include "adevs/adevs.h"
#include "adevs/fmi.h"

class HelloWorld : public adevs::FMI<double> {
  public:
    HelloWorld()
        : adevs::FMI<double>(
              "HelloWorld", "{0b2cba00-281e-4e29-841c-79167ef52ced}",
              "file:///home/1qn/Code/adevs-code/examples/fmi/Example1/"
              "HelloWorld/resources",
              1, 0, "HelloWorld/binaries/linux64/HelloWorld.so") {}
    double get_x() { return get_real(0); }
    void set_x(double val) { set_real(0, val); }
    double get_der_x_() { return get_real(1); }
    void set_der_x_(double val) { set_real(1, val); }
    double get_a() { return get_real(2); }
    void set_a(double val) { set_real(2, val); }
};

#endif

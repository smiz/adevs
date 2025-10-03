#include <iostream>
#include "SimControl.h"


int main() {
    try {
        SimControl control;
        control.run();
    } catch (DisplayException err) {
        std::cerr << err.what() << std::endl;
    }
    return 0;
}

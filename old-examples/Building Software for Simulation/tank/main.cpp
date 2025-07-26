#include <iostream>
#include "SimControl.h"
using namespace std;

int main() {
    try {
        SimControl control;
        control.run();
    } catch (DisplayException err) {
        cerr << err.what() << endl;
    }
    return 0;
}

#include <cmath>
#include <iostream>
#include <vector>
#include "adevs/adevs.h"

using namespace std;

int main(int argc, char** argv) {
    /* plot a polynomial approx to sin(t) and d/dt sin(t) = cos(t) */
    unsigned n = atoi(argv[1]);
    double tend = 6.14;
    double dt = tend / (double)n;

    vector<double> tdat(n);
    vector<double> xdat(n);

    for (unsigned i = 0; i < n; i++) {
        xdat[i] = sin(i * dt);
        tdat[i] = i * dt;
    }

    adevs::InterPoly p(xdat.data(), tdat.data(), n);
    for (double t = 0.0; t <= tend; t += dt / 2.0) {
        cout << t << " " << p(t) << " " << p.derivative(t) << endl;
    }

    return 0;
}

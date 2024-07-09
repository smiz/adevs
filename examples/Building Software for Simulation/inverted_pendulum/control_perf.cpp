#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
    double* p = new double[argc];
    p[0] = 0.0;
    for (int i = 1; i < argc; i++) {
        ifstream fin(argv[i]);
        p[i] = 0.0;
        double t = 0.0, f = 0.0;
        while (true) {
            double tl = t, fl = f;
            fin >> t >> f;
            if (fin.eof()) {
                break;
            }
            p[i] += (t - tl) * ((fl * fl) + (f * f)) / 2.0;
            ;
        }
        fin.close();
        p[0] += p[i];
        cout << p[i] << endl;
    }
    cout << "Avg. " << p[0] / (argc - 1) << endl;
    return 0;
}

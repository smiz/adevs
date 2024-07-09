#include <iostream>
#include "Qn.h"
#include "adevs/par_simulator.h"
using namespace adevs;
using namespace std;

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Need # queues, # lines, and end time" << endl;
        return 0;
    }
    if (argc == 5) {
        cout << "Using " << omp_get_max_threads() << " threads" << endl;
    }
    int queues = atoi(argv[1]);
    int lines = atoi(argv[2]);
    double tend = atof(argv[3]);
    LpGraph lpg;
    Qn* model = new Qn(queues, lines, lpg);
    AbstractSimulator<int>* sim;
    if (argc == 5) {
        sim = new ParSimulator<int>(model, lpg);
    } else {
        sim = new Simulator<int>(model);
    }
    sim->execUntil(tend);
    delete sim;
    delete model;
    return 0;
}

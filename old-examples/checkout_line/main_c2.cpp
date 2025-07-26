#include <iostream>

#include "Clerk2.h"
#include "Generator.h"
#include "Observer.h"

using namespace std;
using namespace adevs;


int main(int argc, char** argv) {

    if (argc != 3) {
        cout << "Need an input file and output file!" << endl;
        return 1;
    }

    // Create the model
    shared_ptr<adevs::Digraph<shared_ptr<Customer>>> store =
        make_shared<adevs::Digraph<shared_ptr<Customer>>>();

    shared_ptr<Clerk2> clerk = make_shared<Clerk2>();
    shared_ptr<Generator> generator = make_shared<Generator>(argv[1]);
    shared_ptr<Observer> observer = make_shared<Observer>(argv[2]);

    store->add(clerk);
    store->add(generator);
    store->add(observer);

    store->couple(generator, generator->arrive, clerk, clerk->arrive);
    store->couple(clerk, clerk->depart, observer, observer->departed);

    // Run the simulation
    Simulator<PortValue<shared_ptr<Customer>>> simulator(store);
    while (simulator.nextEventTime() < DBL_MAX) {
        simulator.execNextEvent();
    }
    return 0;
}

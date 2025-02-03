#include <iostream>

#include "Clerk.h"
#include "Clerk2.h"
#include "Customer.h"
#include "Generator.h"
#include "MultiClerk.h"
#include "Observer.h"
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


int main(int argc, char** argv) {

    if (argc != 3) {
        cout << "Need an input and output file!" << endl;
        return 1;
    }

    shared_ptr<adevs::Digraph<shared_ptr<Customer>>> store =
        make_shared<adevs::Digraph<shared_ptr<Customer>>>();

    shared_ptr<MultiClerk> clerk = make_shared<MultiClerk>();
    shared_ptr<Generator> generator = make_shared<Generator>(argv[1]);
    shared_ptr<Observer> observer = make_shared<Observer>(argv[2]);

    store->add(clerk);
    store->add(generator);
    store->add(observer);

    store->couple(generator, generator->arrive, clerk, clerk->arrive);
    store->couple(clerk, clerk->depart, observer, observer->departed);

    Simulator<EventType> simulator(store);

    while (simulator.nextEventTime() < 100.0) {
        simulator.execNextEvent();
    }
    return 0;
}

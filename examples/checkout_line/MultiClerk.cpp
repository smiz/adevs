#include "MultiClerk.h"
using namespace std;
using namespace adevs;

// Assign identifiers to I/O ports
int const MultiClerk::arrive = 0;
int const MultiClerk::depart = 1;

MultiClerk::MultiClerk() : Digraph<Customer*>() {
    // Create and add component models
    Decision* d = new Decision();
    add(d);
    Clerk* c[NUM_LINES];
    for (int i = 0; i < NUM_LINES; i++) {
        c[i] = new Clerk();
        add(c[i]);
    }
    // Create model connections
    couple(this, this->arrive, d, d->decide);
    for (int i = 0; i < NUM_LINES; i++) {
        couple(d, d->arrive[i], c[i], c[i]->arrive);
        couple(c[i], c[i]->depart, d, d->departures[i]);
        couple(c[i], c[i]->depart, this, this->depart);
    }
}

MultiClerk::~MultiClerk() {}

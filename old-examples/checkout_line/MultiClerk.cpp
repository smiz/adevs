#include "MultiClerk.h"


using namespace adevs;

// Assign identifiers to I/O ports
int const MultiClerk::arrive = 0;
int const MultiClerk::depart = 1;

MultiClerk::MultiClerk() : Digraph<std::shared_ptr<Customer>>() {
    // Create and add component models
    std::shared_ptr<Decision> decision = std::make_shared<Decision>();
    add(decision);
    std::shared_ptr<Clerk> clerks[NUM_LINES];
    for (int ii = 0; ii < NUM_LINES; ii++) {
        clerks[ii] = std::make_shared<Clerk>();
        add(clerks[ii]);
    }

    // Create model connections
    couple_input(this->arrive, decision, decision->decide);
    for (int ii = 0; ii < NUM_LINES; ii++) {
        couple(decision, decision->arrive[ii], clerks[ii], clerks[ii]->arrive);
        couple(clerks[ii], clerks[ii]->depart, decision,
               decision->departures[ii]);
        couple_output(clerks[ii], clerks[ii]->depart, this->depart);
    }
}

MultiClerk::~MultiClerk() {}

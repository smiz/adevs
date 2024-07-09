#ifndef __multi_clerk_h_
#define __multi_clerk_h_
#include "Clerk.h"
#include "Decision.h"
#include "adevs.h"

/**
A model of a store with multiple clerks and a "shortest line"
decision process for customers.
*/
class MultiClerk : public adevs::Digraph<Customer*> {
  public:
    // Model input port
    static int const arrive;
    // Model output port
    static int const depart;
    // Constructor.
    MultiClerk();
    // Destructor.
    ~MultiClerk();
};

#endif

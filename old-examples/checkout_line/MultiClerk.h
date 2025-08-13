#ifndef _multi_clerk_h_
#define _multi_clerk_h_

#include <memory>

#include "Clerk.h"
#include "Decision.h"
#include "adevs/adevs.h"

/**
A model of a store with multiple clerks and a "shortest line"
decision process for customers.
*/
class MultiClerk : public adevs::Digraph<std::shared_ptr<Customer>> {

  public:
    // Model input/output ports
    static int const arrive;
    static int const depart;

    MultiClerk();
    ~MultiClerk();
};

#endif

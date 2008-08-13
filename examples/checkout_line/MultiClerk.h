#ifndef __multi_clerk_h_
#define __multi_clerk_h_
#include "adevs.h"
#include "Clerk.h"
#include "Decision.h"

/**
A model of a store with multiple clerks and a "shortest line"
decision process for customers.
*/
class MultiClerk: public adevs::Digraph<Customer*>
{
	public:
		// Model input port
		static const int arrive;
		// Model output port
		static const int depart;
		// Constructor.
		MultiClerk();
		// Destructor.
		~MultiClerk();
};

#endif

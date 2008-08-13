#ifndef _Factory_h_
#define _Factory_h_
#include "adevs.h"
#include "Machine.h"
#include <list>

/**
 * This class implements the Factory and it machine usage policy.
 */
class Factory: public adevs::Network<int> {
	public:
		Factory();
		void getComponents(adevs::Set<adevs::Devs<int>*>& c);
		void route(const int& order, adevs::Devs<int>* src,
				adevs::Bag<adevs::Event<int> >& r);
		bool model_transition();
		~Factory();
		// Get the number of machines
		int getMachineCount();
	private:
		// This is the machine set
		std::list<Machine*> machines;
		// Method for adding a machine to the factory
		void add_machine();
		// Compute time needed for a machine to finish a new job
		double compute_service_time(Machine* m);
};

#endif

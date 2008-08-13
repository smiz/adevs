#ifndef _observer_h_
#define _observer_h_
#include "adevs.h"
#include "Customer.h"
#include <fstream>

/**
 * The Observer records performance statistics for a Clerk model
 * based on its observable output.
 */
class Observer: public adevs::Atomic<IO_Type>
{
	public:
		/// Input port for receiving customers that leave the store.
		static const int departed;
		/// Constructor. Results are written to the specified file.
		Observer(const char* results_file);
		/// Internal transition function.
		void delta_int();
		/// External transition function.
		void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
		/// Confluent transition function.
		void delta_conf(const adevs::Bag<IO_Type>& xb);
		/// Time advance function.
		double ta();
		/// Output function.  
		void output_func(adevs::Bag<IO_Type>& yb);
		/// Output value garbage collection.
		void gc_output(adevs::Bag<IO_Type>& g);
		/// Destructor.
		~Observer();
	private:	
		/// File for storing information about departing customers.
		std::ofstream output_strm;
}; 

#endif

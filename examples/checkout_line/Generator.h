#ifndef _generator_h_
#define _generator_h_
#include "adevs.h"
#include "Customer.h"
#include <list>

/**
 * This class produces Customers according to the provided schedule.
 */
class Generator: public adevs::Atomic<IO_Type> 
{
	public:
		/// Constructor.
		Generator(const char* data_file);
		/// Internal transition function.
		void delta_int();
		/// External transition function.
		void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
		/// Confluent transition function.
		void delta_conf(const adevs::Bag<IO_Type>& xb);
		/// Output function.  
		void output_func(adevs::Bag<IO_Type>& yb);
		/// Time advance function.
		double ta();
		/// Output value garbage collection.
		void gc_output(adevs::Bag<IO_Type>& g);
		/// Destructor.
		~Generator();
		/// Model output port.
		static const int arrive;

	private:	
		/// List of arriving customers.
		std::list<Customer*> arrivals;
}; 

#endif

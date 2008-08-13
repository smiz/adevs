#ifndef _clerk2_h_
#define _clerk2_h_
#include "adevs.h"
#include "Customer.h"
#include <list>

class Clerk2: public adevs::Atomic<IO_Type> 
{
	public:
		/// Constructor.
		Clerk2();
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
		~Clerk2();
		/// Model input port.
		static const int arrive;
		/// Model output port.
		static const int depart;

	private:	
		/// Structure for storing information about customers in the line
		struct customer_info_t
		{
			// The customer
			Customer* customer;
			// Time remaining to process the customer order
			double t_left;
		};
		/// List of waiting customers.
		std::list<customer_info_t> line;
		//// Time before we can preempt another customer
		double preempt;
		/// The clerk's clock
		double t;
		/// Threshold correspond to a 'small' order processing time
		static const double SMALL_ORDER;
		/// Minimum time between preemptions.
		static const double PREEMPT_TIME;
}; 

#endif

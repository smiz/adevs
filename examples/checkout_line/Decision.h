#ifndef __decision_h_
#define __decision_h_
#include "adevs.h"
#include "Customer.h"
#include <list>

// Number of lines to consider.
#define NUM_LINES 2

class Decision: public adevs::Atomic<IO_Type>
{
	public:
		/// Constructor.
		Decision();
		/// Internal transition function.
		void delta_int();
		/// External transition function.
		void delta_ext(double e, const adevs::Bag<IO_Type>& x);
		/// Confluent transition function.
		void delta_conf(const adevs::Bag<IO_Type>& x);
		/// Output function.  
		void output_func(adevs::Bag<IO_Type>& y);
		/// Time advance function.
		double ta();
		/// Output value garbage collection.
		void gc_output(adevs::Bag<IO_Type>& g);
		/// Destructor.
		~Decision();
		/// Input port that receives new customers
		static const int decide;
		/// Input ports that receive customers leaving the two lines
		static const int departures[NUM_LINES];
		/// Output ports that produce customers for the two lines
		static const int arrive[NUM_LINES];

	private:
		/// Lengths of the two lines
		int line_length[NUM_LINES];
		/// List of deciding customers and their decision.
		std::list<std::pair<int,Customer*> > deciding;
		/// Delete all waiting customers and clear the list.
		void clear_deciders();
		/// Returns the arrive port associated with the shortest line
		int find_shortest_line();
};

#endif

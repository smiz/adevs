#ifndef __cell_h_
#define __cell_h_
#include "adevs.h"

struct value_t
{
	bool value;
	int pos;
};

/// A single cell  
class Cell:
	public adevs::Atomic<adevs::CellEvent<value_t> >
{
	public:
		Cell(int position, const bool* const state, double h);
		// State transition functions
		void delta_int();
		void delta_ext(double e, const adevs::Bag<adevs::CellEvent<value_t> >& xb);
		void delta_conf(const adevs::Bag<adevs::CellEvent<value_t> >& xb);
		// Time advance function
		double ta();
		// Output function
		void output_func(adevs::Bag<adevs::CellEvent<value_t> >& yb);
		// Garbage collection method is not needed for this model
		void gc_output(adevs::Bag<adevs::CellEvent<value_t> >&){}
		// Destructor
		~Cell(){}
		bool getState() { return q; }
		static void setParams(bool* vis, unsigned rule, int length);
	private:	
		static bool* vis;
		static unsigned rule;
		static int length;
		// location of the cell in the 2D space.
		const int pos, left, right;
		const double h;
		// Current cell state
		bool q;
		// Neighboring state
		bool n[2];
		// Time to next event
		double dt;
};

#endif

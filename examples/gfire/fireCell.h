#ifndef __fireCell_h_
#define __fireCell_h_
#include <adevs.h>
#include <iostream>

/// The phase of the cell.
typedef enum 
{ 
	IGNITE, // Cell will ignite and notify its neighbor
	BURN_FAST, // Cell will burn without notifying its neighbors
	BURN, // Cell is burning
	BURNED, // Cell has caught fire and used up all of its fuel
	UNBURNED // Cell has not caught fire yet
} 
Phase;

/**
Cell IO type.  Value is +1 or -1 indicating that neighbors
heat should go up or down.
*/
typedef adevs::CellEvent<int> CellEvent;

/**
A cell in the simple forest fire model.  The cell catches fire
when at least two of its neighbors are burning.  The cell continues
to burn until it is out of fuel.
*/
class fireCell: public adevs::Atomic<CellEvent>
{
	public:
		// Create the cell and set the initial conditions
		fireCell(double fuel, bool on_fire, long int x, long int y);
		// State initialization function
		void init();
		// Internal transition function
		void delta_int();
		// External transition function
		void delta_ext(double e, const adevs::Bag<CellEvent>& xb);
		// Confluent transition function
		void delta_conf(const adevs::Bag<CellEvent>& xb);
		// Output function
		void output_func(adevs::Bag<CellEvent>& yb);
		// Time advance function
		double ta();
		// Garbage collection. Does nothing.
		void gc_output(adevs::Bag<CellEvent>& g){}
		// Destructor
		~fireCell(){}
		// Cell phase of a the state
		Phase getPhase();
		// Get the cell x location
		long int xpos() const { return x; }
		// get the cell y location
		long int ypos() const { return y; }
	protected:
		// The phase of the cell
		Phase phase;
		// Amount of fuel available in the cell
		double fuel;
		// Number of burning neighbor cells
		int heat;
		// Cell location
		const long int x, y;
		// Time required for the fire to spread to a neighbor
		static const double move_rate;
};

#endif

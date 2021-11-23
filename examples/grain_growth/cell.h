#ifndef __cell_h_
#define __cell_h_
#include <adevs.h>
#include <iostream>
#include <random>

#define SIZE 800

/**
 * Each integer index is associated with an angle. The angle table
 * is created prior to the simulation start. An entry -1 is no
 * angle.
 */
typedef adevs::CellEvent<int> CellEvent;

/**
 * A lattice point in our grain growth model.
 */
class Cell: public adevs::Atomic<CellEvent>
{
	public:
		static int state_changes;
		// Random number generator for transition times
		static std::default_random_engine generator;		
		static std::exponential_distribution<double> distribution;
		// Orientation map. This is the model initial condition and output.
		static int angle[SIZE][SIZE];
		// Create the cell and set the initial conditions
		Cell(int x, int y);
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
		~Cell(){}
		// Get the grain angle
		int getAngle() const { return angle[x][y]; }
		// Get the cell x location
		int xpos() const { return x; }
		// get the cell y location
		int ypos() const { return y; }
	private:
		// New angle to reduce energy
		int new_angle;
		// Cell location
		const int x, y;
		// Time to go
		double q;

		void calc_next();
};

#endif

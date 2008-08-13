#ifndef __cell_h_
#define __cell_h_
#include "adevs.h"

/// Possible cell phases.
typedef enum { Dead, Alive } Phase;
/// IO type for a cell
typedef adevs::CellEvent<Phase> CellEvent;

/// A cell in the Game of Life.  
class Cell: public adevs::Atomic<CellEvent>
{
	public:
		/**
		Create a cell and set the initial state.
		The width and height fields are used to determine if a
		cell is an edge cell.  The last phase pointer is used to
		visualize the cell space.
		*/
		Cell(long int x, long int y, long int width, long int height, 
		Phase phase, short int nalive, Phase* vis_phase = NULL);
		// State transition functions
		void delta_int();
		void delta_ext(double e, const adevs::Bag<CellEvent>& xb);
		void delta_conf(const adevs::Bag<CellEvent>& xb);
		// Time advance function
		double ta();
		// Output function
		void output_func(adevs::Bag<CellEvent>& yb);
		// Garbage collection method is not needed for this model
		void gc_output(adevs::Bag<CellEvent>& g){}
		// Destructor
		~Cell(){}

	private:	
		// location of the cell in the 2D space.
		long int x, y;
		// dimensions of the 2D space
		static long int w, h;
		// Current cell phase
		Phase phase;
		// number of living neighbors.
		short int nalive;
		// Output variable for visualization
		Phase* vis_phase;

		// Returns true if the cell will be born
		bool check_born_rule() const
		{
			return (phase == Dead && nalive == 3);
		}
		// Return true if the cell will die
		bool check_death_rule() const
		{
			return (phase == Alive && (nalive < 2 || nalive > 3));
		}
};

#endif

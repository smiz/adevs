#ifndef __cell_h_
#define __cell_h_
#include "adevs.h"

/// Length of the rule array
#define RULE_ARRAY_LEN 64
/// Possible cell phases.
typedef enum { Dead = 0, Alive = 1} Phase;
/// IO type for a cell
typedef adevs::CellEvent<Phase> CellEvent;

/// A cell in the Game of Life.  
class Cell: public adevs::Atomic<CellEvent,int>
{
	public:
		/**
		Create a cell and set the initial state.
		The width and height fields are used to determine if a
		cell is an edge cell.  The last phase pointer is used to
		visualize the cell space.
		*/
		Cell(int x, int y, Phase phase, Phase* vis_phase = NULL);
		// State transition functions
		void delta_int();
		void delta_ext(int e, const adevs::Bag<CellEvent>& xb);
		void delta_conf(const adevs::Bag<CellEvent>& xb);
		// Time advance function
		int ta();
		// Output function
		void output_func(adevs::Bag<CellEvent>& yb);
		// Garbage collection method is not needed for this model
		void gc_output(adevs::Bag<CellEvent>& g){}
		// Destructor
		~Cell(){}
		// Initialize the global variables
		static void init_space(int w, int h, const uint8_t* rule);
	private:	
		// location of the cell in the 2D space.
		const int x, y;
		// dimensions of the 2D space
		static int w, h;
		// Output space
		static Phase** y_space;
		// Rule array
		static bool rule[512];
		// Current cell phase
		Phase phase;
		// Output variable for visualization
		Phase* vis_phase;

		unsigned rule_index();	

};

#endif

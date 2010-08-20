#ifndef LookLeft_h_
#define LookLeft_h_
#include "adevs.h"

#define BLACK 0
#define WHITE 1

// A left looking event automaton. The CellEvent structure
// contains an integer that is the cell's position and 
// another integer that has its state.
class LookLeft: public adevs::Atomic<adevs::CellEvent<int> >
{
	public:
		// Constructor puts the cell into its initial state of
		// black if the location is even and white otherwise
		LookLeft(int width, int location, double P):
			adevs::Atomic<adevs::CellEvent<int> >(),
			location(location),width(width),P(P)
		{
			s = l = WHITE;
			// We have a state 0
			if (location % 2 == 0) s = BLACK;
			else l = BLACK; // or our neighbor does
			c = 0.0;
		}
		void delta_int() { c = 0.0; s = l; }
		void delta_ext(double e, const adevs::Bag<adevs::CellEvent<int> >& xb)
		{
			c += e;
			l = (*(xb.begin())).value;
		}
		void delta_conf(const adevs::Bag<adevs::CellEvent<int> >& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		double ta() { return P-c; }
		// Output function
		void output_func(adevs::Bag<adevs::CellEvent<int> >& yb)
		{
			adevs::CellEvent<int> y;
			y.x = (location+1)%width;
			y.value = l;
			yb.insert(y);
		}
		// The gc method is not needed
		void gc_output(adevs::Bag<adevs::CellEvent<int> >&){}
		// Get the location of the cell
		int getLocation() const { return location; }
		// Get the state of the cell
		int getState() const { return s; }
	private:
		int s, l; // Own and left neighbor's discrete state
		double c; // Time spent in the own, current discrete state
		const int location, width; // Our postion in the space and its size
		const double P; // Time interval between changes of s
};

#endif

#include "Cell.h"
using namespace std;

// Cellspace dimensions
long int Cell::w = 0;
long int Cell::h = 0;

Cell::Cell(long int x, long int y, long int w, long int h, 
Phase phase, short int nalive, Phase* vis_phase):
adevs::Atomic<CellEvent>(),
x(x),
y(y),
phase(phase),
nalive(nalive),
vis_phase(vis_phase)
{
	// Set the global cellspace dimensions
	Cell::w = w;
	Cell::h = h;
	// Set the initial visualization value
	if (vis_phase != NULL) *vis_phase = phase;
}

double Cell::ta()
{
	// If a phase change should occur
	if (check_death_rule() // cell will die
	|| check_born_rule()) // cell will be birthed
	{
		return 1.0;
	}
	// Otherwise, do nothing
	return DBL_MAX;
}

void Cell::delta_int() 
{
	// Change the cell state if necessary
	if (check_death_rule())
	{
		phase = Dead;
	}
	else if (check_born_rule())
	{
		phase = Alive;
	}
}

void Cell::delta_ext(double e, const adevs::Bag<CellEvent>& xb) 
{
	// Update the count if living neighbors
	adevs::Bag<CellEvent>::const_iterator iter;
	for (iter = xb.begin(); iter != xb.end(); iter++)
	{
		if ((*iter).value == Dead) nalive--;
		else nalive++;
	}
}

void Cell::delta_conf(const adevs::Bag<CellEvent>& xb) 
{
	delta_int();
	delta_ext(0.0,xb);
}

void Cell::output_func(adevs::Bag<CellEvent>& yb) 
{
	CellEvent e;
	// Assume we are dying
	e.value = Dead;
	// Check in case this in not true
	if (check_born_rule())
	{
		e.value = Alive;
	}
	// Set the initial visualization value
	if (vis_phase != NULL) *vis_phase = e.value;
	// Generate an event for each neighbor
	for (long int dx = -1; dx <= 1; dx++)
	{
		for (long int dy = -1; dy <= 1; dy++)
		{
			e.x = (x+dx)%w;
			e.y = (y+dy)%h;
			if (e.x < 0) e.x = w-1;
			if (e.y < 0) e.y = h-1;
			// Don't send to self
			if (e.x != x || e.y != y)
			{
				yb.insert(e);
			}
		}
	}
}

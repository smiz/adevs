#include "cell.h"
#include <cassert>
#include <iostream>
using namespace std;

int Cell::state_changes = 0;
default_random_engine Cell::generator;
exponential_distribution<double> Cell::distribution(1.0);

int Cell::angle[SIZE][SIZE];

Cell::Cell(int x, int y): 
	adevs::Atomic<CellEvent>(),
	x(x),
	y(y),
	q(adevs_inf<double>())
{
	calc_next();
}

double Cell::ta()
{
	return q;
}

void Cell::delta_int()
{
	q = adevs_inf<double>();
	calc_next();
	state_changes++;
}

void Cell::delta_ext(double e, const adevs::Bag<CellEvent>& xb)
{
	if (q < adevs_inf<double>())
		q -= e;
	calc_next();
	state_changes++;
}

void Cell::delta_conf(const adevs::Bag<CellEvent>& xb) 
{
	q = adevs_inf<double>();
	calc_next();
	state_changes++;
}

void Cell::output_func(adevs::Bag<CellEvent>& yb)
{
	CellEvent e;
	e.value = angle[x][y] = new_angle;
	// Produce an event for each of the 8 neighbors
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			e.x = x+dx;
			e.y = y+dy;
			// Don't send an event to self
			if (e.x != x || e.y != y)
			{
				yb.insert(e);
			}
		}
	}
}

void Cell::calc_next()
{
	int E = 9; // Worst case energy 
	new_angle = -1;
	// Find lowest energy choice
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			int nx = x+dx, ny = y+dy;
			if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
			{
				int trial_angle = angle[nx][ny], trial_E = 0;
				if (trial_angle == new_angle) continue;
				for (int ddx = -1; ddx <= 1; ddx++)
				{
					for (int ddy = -1; ddy <= 1; ddy++)
					{
						if (ddx != 0 || ddy != 0)
						{
							int nnx = x+ddx, nny = y+ddy;
							if (nnx >= 0 && nnx < SIZE && nny >= 0 && nny < SIZE)
							{
								if (trial_angle != angle[nnx][nny])
									trial_E++;
							}
						}
					}
				}
				if (trial_E < E || (trial_E == E && trial_angle == angle[x][y]))
				{
					E = trial_E;
					new_angle = trial_angle;
				}
			}
		}
	}
	// No change. Sit tight.
	if (new_angle == angle[x][y])
		q = adevs_inf<double>();
	// Change but still active
	else if (q == adevs_inf<double>())
		q = distribution(generator);
}

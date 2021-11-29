#include "cell.h"
#include <cassert>
#include <iostream>
#include <climits>
using namespace std;

int Cell::state_changes = 0;
default_random_engine Cell::generator;
exponential_distribution<double> Cell::distribution(W);

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
	if (angle[x][y] == new_angle) return;
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
	int Eo[9];
	int En[9];
	int Ec[9];
	int k = 0, min_E = INT_MAX;
	int options = 0, unique_options = 0;
	// Find lowest energy choice
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			int nx = x+dx, ny = y+dy;
			if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
			{
				Ec[k] = angle[nx][ny];
				En[k] = 0;
				for (int ddx = -1; ddx <= 1; ddx++)
				{
					for (int ddy = -1; ddy <= 1; ddy++)
					{
						if (ddx == 0 && ddy == 0) continue;
						int nnx = x+ddx, nny = y+ddy;
						if (nnx >= 0 && nnx < SIZE && nny >= 0 && nny < SIZE)
						{
							if (Ec[k] != angle[nnx][nny])
								En[k]++;
						}
					}
				}
				min_E = ::min(min_E,En[k]);
				k++;
			}
		}
	}
	for (int i = 0; i < k; i++)
	{
		if (min_E == En[i])
		{
			int j;
			for (j = 0; j < options; j++)
			{
				if (Ec[i] == Eo[j])
					break;
			}
			if (j == options || options == 0)
				unique_options++;
			Eo[options] = Ec[i];
			options++;
		}
	}
	new_angle = Eo[rand()%options];
	if (unique_options == 1 && new_angle == angle[x][y])
		q = adevs_inf<double>();
	// Change but still active
	else if (q == adevs_inf<double>())
		q = distribution(generator);
}

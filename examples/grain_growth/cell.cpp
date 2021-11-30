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

static int neighbors[8][2] =
{
	{0,1},
	{0,-1},
	{1,0},
	{-1,0},
	{1,1},
	{1,-1},
	{-1,1},
	{-1,-1}
};

int Cell::energy(int C)
{
	int E = 0;
	for (int i = 0; i < 8; i++)
	{
		int nx = x+neighbors[i][0];
		int ny = y+neighbors[i][1];
		if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
		{
			if (C != angle[nx][ny])
				E++;
		}
	}
	return E;
}

void Cell::calc_next()
{
	bool options = false;
	int option_count = 0, px, py;
	int En[8]; 
	int C[8];
	int E = energy(angle[x][y]);
	for (int i = 0; i < 8; i++)
	{
		px = x+neighbors[i][0];
		py = y+neighbors[i][1];
		if (px >= 0 && px < SIZE && py >= 0 && py < SIZE)
		{
			En[option_count] = energy(angle[px][py]);
			C[option_count] = angle[px][py];
			options = options || (En[option_count] <= E && C[option_count] != angle[x][y]);
			option_count++;
		}
	}
	// No viable alternatives to current state
	if (!options)
	{
		q = adevs_inf<double>();
		return;
	}
	int pick = rand()%option_count;
	if (En[pick] < E || (En[pick] == E && rand()%2 == 0))
		new_angle = C[pick];
	else
		new_angle = angle[x][y];
	if (q == adevs_inf<double>())
		q = distribution(generator);
}

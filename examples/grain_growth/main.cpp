#include "cell.h"
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cstring>
#include "save_results.h"
#include "params.h"
using namespace std;

double tend;

void simulateSpace()
{	
	srand(0);
	int *grains = new int[SIZE*SIZE];
	// Dynamic cellspace model and simulator
	adevs::CellSpace<int>* cell_space = NULL;
	adevs::Simulator<CellEvent>* sim = NULL;
	int k = 0;
	// Create a simulator if needed
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			Cell::angle[i][j] = k++;
	cell_space = 
			new adevs::CellSpace<int>(SIZE,SIZE);
	// Create a model to go into each point of the cellspace
	for (int x = 0; x < SIZE; x++)
	{
		for (int y = 0; y < SIZE; y++)
		{
			Cell* cell = new Cell(x,y);
			cell_space->add(cell,x,y);
		}
	}
	// Create a simulator for the model
	sim = new adevs::Simulator<CellEvent>(cell_space);
	double tL = 0.0;
	clock_t start = clock();
	while (sim->nextEventTime() < adevs_inf<double>() && sim->nextEventTime() < tend)
	{
		tL = sim->nextEventTime();
		sim->execNextEvent();
	}
	cout << tL << " " << clock()-start << " " << Cell::state_changes << endl;
	k = 0;
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			grains[k] = Cell::angle[i][j];
			k++;
		}
	}
	save_results(grains,SIZE,SIZE,"des.png");
	delete sim;
	delete cell_space;
	delete [] grains;
}

int main(int argc, char** argv)
{
	tend = atof(argv[1]);
	simulateSpace();
	// Done
	return 0;
}

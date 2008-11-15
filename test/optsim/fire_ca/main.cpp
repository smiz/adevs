#include "fireCell.h"
#include "Configuration.h"
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
using namespace std;

// Use the parallel simulator?
static bool par_sim = false;
// Phase space to visualize
fireCell::state_t** snap_shot = NULL;
Configuration* config = NULL;

class CellListener:
	public adevs::EventListener<CellEvent>
{
	public:
		void stateChange(adevs::Atomic<CellEvent>* model, double t, void* state)
		{
			fireCell* cell = dynamic_cast<fireCell*>(model);
			snap_shot[cell->xpos()][cell->ypos()] = cell->getState(state);
		}
		void outputEvent(adevs::Event<CellEvent>,double){}
};

void dumpState(int k)
{
	char fname[100];
	if (par_sim)
		sprintf(fname,"soln_%d_p",k);
	else
		sprintf(fname,"soln_%d",k);
	ofstream fout(fname);
	// Draw all of the cells
	for (int x = 0; x < config->get_width(); x++)
	{
		for (int y = 0; y < config->get_height(); y++)
		{
			fout << snap_shot[x][y].phase << " " <<
				snap_shot[x][y].fuel << " " <<
				snap_shot[x][y].heat << endl;
		}
	}
	fout.close();
}
 
void simulateSpace()
{
	// Cellspace model and simulator
	adevs::CellSpace<int>* cell_space = NULL;
	adevs::Simulator<CellEvent>* sim = NULL;
	adevs::OptSimulator<CellEvent>* opt_sim = NULL;
	CellListener* listener = NULL;
	// snap shot data
	if (snap_shot == NULL)
	{
		snap_shot = new fireCell::state_t*[config->get_width()];
		for (int x = 0; x < config->get_width(); x++)
		{
			snap_shot[x] = new fireCell::state_t[config->get_height()];
		}
	}
	// Create a simulator 
	cell_space = 
		new adevs::CellSpace<int>(config->get_width(),config->get_height());
	// Create a model to go into each point of the cellspace
	for (int x = 0; x < config->get_width(); x++)
	{
		for (int y = 0; y < config->get_height(); y++)
		{
			fireCell* cell = new fireCell(config->get_fuel(x,y),
				config->get_fire(x,y),x,y);
			cell_space->add(cell,x,y);
			snap_shot[x][y] = cell->getState();
		}
	}
	// Create a listener for the model
	listener = new CellListener();
	// Create a simulator for the model
	if (!par_sim)
	{
		sim = new adevs::Simulator<CellEvent>(cell_space);
		// Add an event listener
		sim->addEventListener(listener);
	}
	else
	{
		opt_sim = new adevs::OptSimulator<CellEvent>(cell_space,100,true);
		// Add an event listener
		opt_sim->addEventListener(listener);
	}
	// Run the next simulation step
	try
	{
		for (int i = 0; i < 10; i++)
		{
			cout << "computing snapshot " << i << endl;
			if (par_sim) opt_sim->execUntil(i*50.0);
			else while (sim->nextEventTime() <= i*50.0)
				sim->execNextEvent();
			cout << "writing snapshot " << i << endl;
			dumpState(i);
		}
	} 
	catch(adevs::exception err)
	{
		cout << err.what() << endl;
		exit(-1);
	}
	if (sim != NULL) delete sim;
	if (opt_sim != NULL) delete opt_sim;
	delete cell_space;
	delete listener;
	for (int x = 0; x < config->get_width(); x++)
	{
		delete [] snap_shot[x];
	}
	delete [] snap_shot;
}

int main(int argc, char** argv)
{
	// Load the initial fire model data
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i],"--config") == 0 && i+1 < argc)
		{
			config = new Configuration(argv[++i]);
		}
		else if (strcmp(argv[i],"-p") == 0)
		{
			par_sim = true;
			int procs = omp_get_num_procs();
			int thrds = omp_get_max_threads();
			cout << "Using " << thrds << " threads on " << procs << " processors" << endl;
		}
	}
	if (config == NULL)
	{
		cout << "No data" << endl; 
		return 0;
	}
	simulateSpace();
	// Done
	return 0;
}

#include "fireCell.h"
#include "Configuration.h"
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <sys/time.h>
using namespace std;

// Use the parallel simulator?
static bool par_sim = false;
// Produce output? Turn off for performance analysis
static bool no_output = false;
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
	adevs::AbstractSimulator<CellEvent>* sim = NULL;
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
	// Create a simulator for the model
	if (!par_sim)
	{
		sim = new adevs::Simulator<CellEvent>(cell_space);
		opt_sim = NULL;
	}
	else
	{
		sim = opt_sim = new adevs::OptSimulator<CellEvent>(cell_space);
	}
	// Create a listener for the model
	listener = new CellListener();
	sim->addEventListener(listener);
	// Run the next simulation step
	try
	{
		if (no_output)
		{
			sim->execUntil(500.0);
		}
		else for (int i = 0; i < 10; i++)
		{
			cout << "computing snapshot " << i << endl;
			sim->execUntil(i*50.0);
			cout << "writing snapshot " << i << endl;
			dumpState(i);
		}
	} 
	catch(adevs::exception err)
	{
		cout << err.what() << endl;
		exit(-1);
	}
	if (opt_sim != NULL)
	{
		cout << "early output = " << opt_sim->getEarlyOutputCount() << endl; 
		cout << "in time output = " << opt_sim->getInTimeOutputCount() << endl; 
		cout << "ext stalls = " << opt_sim->getExtStallCount() << endl; 
		cout << "int stalls = " << opt_sim->getIntStallCount() << endl; 
	}
	if (sim != NULL) delete sim;
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
		}
		else if (strcmp(argv[i],"--no-output") == 0)
		{
			no_output = true;
		}
	}
	if (config == NULL)
	{
		cout << "No data" << endl; 
		return 0;
	}
	struct timeval tstart, tend;
	gettimeofday(&tstart,NULL);
	simulateSpace();
	gettimeofday(&tend,NULL);
	// Done
	long secs = tend.tv_sec-tstart.tv_sec;
	long usecs = tend.tv_usec-tstart.tv_usec;
	if (usecs < 0) { secs--; usecs += 1000000; }
	cout << secs << " " << usecs << endl;
	return 0;
}

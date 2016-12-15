#include "fireCell.h"
#include "Configuration.h"
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cstring>
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
		void stateChange(adevs::Atomic<CellEvent>* model, double t)
		{
			fireCell* cell = dynamic_cast<fireCell*>(model);
			snap_shot[cell->xpos()][cell->ypos()] = cell->getState();
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
	adevs::ParSimulator<CellEvent>* opt_sim = NULL;
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
		/**
		 * Restrict the number of threads to two.
		 * Otherwise, the performance
		 * is so horrible that you will age waiting
		 * for this test cast to finish.
		 */
		int threads = 2;
		adevs::LpGraph g;
		for (int i = 1; i < threads; i++)
		{
			g.addEdge(i-1,i);
			g.addEdge(i,i-1);
		}
		sim = opt_sim = new adevs::ParSimulator<CellEvent>(cell_space,g);
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
	double tstart = omp_get_wtime();
	simulateSpace();
	double tend = omp_get_wtime();
	// Done
	cout << (tend-tstart) << endl;
	return 0;
}

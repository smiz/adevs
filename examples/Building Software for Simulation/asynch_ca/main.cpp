#include "adevs.h"
#include "Display.h"
#include "LookLeft.h"
#include <fstream>
#include <cmath>
using namespace std;
using namespace adevs;

CellSpace<int>* space;
Display* display;
Simulator<CellEvent<int> >* sim;
double t_max, dt, tL, t_start;
int num_cells;

void draw()
{
	double t = tL-t_start;
	if (t < 0.0) return;
	while (t <= sim->nextEventTime()-t_start)
	{
		for (int k = 0; k < num_cells; k++)
		{
			LookLeft* cell = dynamic_cast<LookLeft*>(space->getModel(k));
			// Draw a white cell (red = green = blue = 255)
			if (cell->getState() == WHITE) 
				display->setColor(t/dt,cell->getLocation(),255,255,255);
			// or draw a black cell (red = green = blue = 0)
			else 
				display->setColor(t/dt,cell->getLocation(),0,0,0);
		}
		if (t == sim->nextEventTime()-t_start) break;
		else if (t + dt > sim->nextEventTime()-t_start) t = sim->nextEventTime()-t_start;
		else t += dt;
	}
	display->checkEvents();
	display->redraw();
}

// The main simulation program
int main(int argc, char** argv)
{
	// Get the number of generations to run the simulation
	if (argc != 4) {
		cout << "Requires # cells, end time, start record time" << endl;
		return 0;
	}
	num_cells = atoi(argv[1]);
	t_max  = atof(argv[2]);
	t_start = atof(argv[3]);
	dt = (t_max-t_start) / 1000.0;
	// Create the display
	display = new Display((int)((t_max-t_start)/dt),num_cells);
	// Create the cellular automaton 
	space = new CellSpace<int>(num_cells);
	unsigned seed = /*time(NULL)*/1229172254 ;
	srand(seed);
	// Create the cells with one black cell at the center
	for (int i = 0; i < num_cells; i++) {
		double P = 1.0/(double)(rand()%3+1);
		P = pow(2.0,P); // Use this for asynchronous automata
//		double P = 1.0; Use this for synchronous automata
		LookLeft* cell = new LookLeft(num_cells,i,P);
		// Add it to the automata
		space->add(cell,i);
	}
	// Run the simulation
	sim = new Simulator<CellEvent<int> >(space);
	tL = 0.0;
	while (sim->nextEventTime() <= t_max)
	{
		draw();
		tL = sim->nextEventTime();
		sim->execNextEvent();
	}
	// Save the final image to a file
	display->toBmp("image.bmp");
	// Clean up and exit
	delete sim; delete space; delete display;
	return 0;
}

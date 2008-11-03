#include "fireCell.h"
#include "Configuration.h"
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <GL/glut.h>
using namespace std;

// Phase space to visualize
Phase** phase = NULL;
Configuration* config = NULL;
double max_init_fuel = 0.0;
// Window and cell dimensions. 
const GLint cell_size = 6;
GLint win_height, win_width;
// Read to run?
static bool phase_data_ready = false;

class PhaseListener:
	public adevs::EventListener<CellEvent>
{
	public:
		void stateChange(adevs::Atomic<CellEvent>* model, double t, void* state)
		{
			fireCell* cell = dynamic_cast<fireCell*>(model);
			phase[cell->xpos()][cell->ypos()] = cell->getPhase(state);
		}
		void outputEvent(adevs::Event<CellEvent>,double){}
};

// Create a random configuration
void random_config()
{
	// Dimensions of the random space
	int dim = 100;
	// Create a temporary configuration file
	char tmpname[100];
	sprintf(tmpname,"fire_config_XXXXXX");
	errno = 0;
	FILE* fout = fopen(tmpname,"w");
	if (errno != 0)
	{
		perror("Could create temporary config file");
		exit(-1);
	}
	// Write the config file 
	fprintf(fout,"width %d\nheight %d\nfuel\n",dim,dim);
	// Assign random amounts of fuel
	for (int i = 0; i < dim*dim; i++)
	{
		double fuel = 10.0*((double)rand()/(double)RAND_MAX);
		fprintf(fout,"%f\n",fuel);
	}
	// Create the initial fire
	fprintf(fout,"fire\n");
	int start = rand()%(dim*dim);
	bool burn = false;
	for (int i = 0; i < dim*dim; i++)
	{
		if (i == start) burn = true;
		else if (burn == true) burn = (rand()%2 == 0);
		if (burn) fprintf(fout,"1\n");
		else fprintf(fout,"0\n");
	}
	// Create a configuration object from the temporary file
	config = new Configuration(tmpname);
	// Close and delete the temporary file
	fclose(fout);
}

void drawSpace()
{
	// Initialize the OpenGL view
	static bool init = true;
	if (init)
	{
		init = false;
		glutUseLayer(GLUT_NORMAL);
		glClearColor(0.0,0.0,1.0,1.0);
		glColor3f(0.0,1.0,0.0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0,(float)win_width,0.0,(float)win_height,1.0,-1.0);
	}
	// Clear the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Is there anything to draw?
	if (!phase_data_ready) return;
	// Draw all of the cells
	for (int x = 0; x < config->get_width(); x++)
	{
		for (int y = 0; y < config->get_height(); y++)
		{
			if (phase[x][y] == BURN || phase[x][y] == BURN_FAST) 
			{
				glColor3f(1.0,0.0,0.0);
			}
			else if (phase[x][y] == BURNED)
			{
				glColor3f(0.0,0.0,0.0);
			}
			else
			{
				float intensity = config->get_fuel(x,y)/max_init_fuel;
				glColor3f(0.0,intensity,0.0);
			}
			GLint wx = cell_size*x;
			GLint wy = cell_size*y;
			glRecti(wx,wy,wx+cell_size,wy+cell_size);
		}
	}
	// Display the new image
	glutSwapBuffers();
}
 
void simulateSpace()
{
	// Dynamic cellspace model and simulator
	static adevs::CellSpace<int>* cell_space = NULL;
	static adevs::Simulator<CellEvent>* sim = NULL;
	static PhaseListener* listener = NULL;
	// If the visualization array is needed
	if (phase == NULL)
	{
		phase = new Phase*[config->get_width()];
		for (int x = 0; x < config->get_width(); x++)
		{
			phase[x] = new Phase[config->get_height()];
		}
	}
	// Create a simulator if needed
	if (cell_space == NULL)
	{
		cell_space = 
			new adevs::CellSpace<int>(config->get_width(),config->get_height());
		// Create a model to go into each point of the cellspace
		for (int x = 0; x < config->get_width(); x++)
		{
			for (int y = 0; y < config->get_height(); y++)
			{
				fireCell* cell = new fireCell(config->get_fuel(x,y),
					config->get_fire(x,y),x,y);
				max_init_fuel = max(max_init_fuel,config->get_fuel(x,y));
				cell_space->add(cell,x,y);
			}
		}
		// Create a simulator for the model
		sim = new adevs::Simulator<CellEvent>(cell_space);
		// Add an event listener
		listener = new PhaseListener();
		sim->addEventListener(listener);
		// Ready to go
		phase_data_ready = true;
	}
	// If everything has died, then restart on the next call
	if (sim->nextEventTime() == DBL_MAX)
	{
		phase_data_ready = false;
		delete cell_space;
		delete sim;
		delete listener;
		sim = NULL;
		cell_space = NULL;
		listener = NULL;
	}
	// Run the next simulation step
	else
	{
		try
		{
			sim->execNextEvent();
		} 
		catch(adevs::exception err)
		{
			cout << err.what() << endl;
			exit(-1);
		}
	}
	// Draw the updated display
	drawSpace();
}

int main(int argc, char** argv)
{
	// Load the initial fire model data
	if (argc >= 2)
	{
		config = new Configuration(argv[1]);
	}
	else
	{
		random_config();
	}
	// Setup the display
	win_height = config->get_height()*cell_size;
	win_width = config->get_width()*cell_size;
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(win_width,win_height);
	glutCreateWindow("gfire");
	glutPositionWindow(0,0);
	glutDisplayFunc(drawSpace);
	glutIdleFunc(simulateSpace);
	glutMainLoop();
	// Done
	return 0;
}

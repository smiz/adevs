#include "adevs.h"
#include "Cell.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <GL/glut.h>
using namespace std;
using namespace adevs;

// Cellspace dimensions
#define WIDTH 300
#define HEIGHT 300
bool state[HEIGHT+1][WIDTH];
unsigned lines = 0, rule, numSteps;

// Window and cell dimensions. 
#define CELL_SIZE 3
const GLint win_width = WIDTH*CELL_SIZE; 
const GLint win_height = HEIGHT*CELL_SIZE;

void drawSpace()
{
	static bool init = true;
	if (init)
	{
		init = false;
		glutUseLayer(GLUT_NORMAL);
		glClearColor(0.0,0.0,1.0,1.0);
		glColor3f(1.0,1.0,1.0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0,(float)win_width,0.0,(float)win_height,1.0,-1.0);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int which_row = lines%HEIGHT;
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			if (state[which_row][x]) 
			{
				GLint wx = CELL_SIZE*x;
				GLint wy = CELL_SIZE*y;
				glRecti(wx,wy,wx+CELL_SIZE,wy+CELL_SIZE);
			}
		}
		which_row--;
		if (which_row == -1) which_row = HEIGHT-1;
	}
	glutSwapBuffers();
}

void simulateSpace()
{
	// Dynamic cellspace model and simulator
	static CellSpace<value_t>* cell_space = NULL;
	static Simulator<CellEvent<value_t> >* sim = NULL;
	// Reset the space if everything has died
	if (cell_space == NULL)
	{
		lines = 0;
		// Create the cell state variable array
		for (int x = 0; x < WIDTH; x++)
		{
			state[HEIGHT][x] = state[0][x] = rand()%2;
			for (int y = 1; y < HEIGHT; y++)
				state[y][x] = false;
		}
		double* steps = new double[numSteps];
		for (unsigned i = 0; i < numSteps; i++)
			steps[i] = (double)rand()/(double)RAND_MAX;
		Cell::setParams(state[HEIGHT],rule,WIDTH);
		// Create the cellspace model
		cell_space = new adevs::CellSpace<value_t>(WIDTH);
		for (int x = 0; x < WIDTH; x++)
		{
			cell_space->add(
				new Cell(x,state[HEIGHT],steps[rand()%numSteps]),x);
		}
		delete [] steps;
		// Create a simulator for the model
		sim = new adevs::Simulator<CellEvent<value_t> >(cell_space);
	}
	// Run the next simulation step
	lines++;
	sim->execNextEvent();
	memcpy(state[lines%HEIGHT],state[HEIGHT],sizeof(bool)*WIDTH);
	// Draw the updated display
	drawSpace();
}

int main(int argc, char** argv)
{
	int seed = atoi(argv[1]);
	rule = atoi(argv[2]);
	numSteps = atoi(argv[3]);
	cout << seed << " " << rule << " " << numSteps << endl;
	// Seed the random number generator
	srand(seed);
	// Setup the display
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(win_width,win_height);
	glutCreateWindow("Async CA");
	glutPositionWindow(0,0);
	glutDisplayFunc(drawSpace);
	glutIdleFunc(simulateSpace);
	glutMainLoop();
	// Done
	return 0;
}

#include "cell.h"
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <map>
#include <GL/glut.h>
using namespace std;

#define MAX_ANGLES 4

struct rgb_t
{
	float r, g, b;
};

map<int,rgb_t> colors;

// Window and cell dimensions. 
const GLint cell_size = 2;
GLint win_height = SIZE*cell_size, win_width = SIZE*cell_size;
bool ready = false;

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
	if (!ready) return;
	// Draw all of the cells
	for (int x = 0; x < SIZE; x++)
	{
		for (int y = 0; y < SIZE; y++)
		{
			if (Cell::angle[x][y] < 0) 
			{
				glColor3f(0.0,0.0,0.0);
			}
			else
			{
				rgb_t color; 
				auto entry = colors.find(Cell::angle[x][y]);
				if (entry != colors.end())
					color = (*entry).second;
				else
				{
					color.r = double(rand())/double(RAND_MAX);
					color.g = double(rand())/double(RAND_MAX);
					color.b = double(rand())/double(RAND_MAX);
					colors[Cell::angle[x][y]] = color;
				}
				glColor3f(color.r,color.g,color.b);
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
	static double tN = adevs_inf<double>();
	// Create a simulator if needed
	if (cell_space == NULL)
	{
		ready = true;
		for (int i = 0; i < SIZE; i++)
			for (int j = 0; j < SIZE; j++)
				Cell::angle[i][j] = rand()%MAX_ANGLES;
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
	}
	// If everything has died, then restart on the next call
	tN = sim->nextEventTime();
	if (tN == adevs_inf<double>())
	{
		delete sim;
		delete cell_space;
		sim = NULL;
		cell_space = NULL;
		ready = false;
		cout << Cell::state_changes << " RESET!" << endl;
		Cell::state_changes = 0;
	}
	else
	{
		sim->execUntil(tN+1);
		drawSpace();
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(win_width,win_height);
	glutCreateWindow("grain");
	glutPositionWindow(0,0);
	glutDisplayFunc(drawSpace);
	glutIdleFunc(simulateSpace);
	glutMainLoop();
	// Done
	return 0;
}

#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Cell.h"
#include "adevs/adevs.h"

using namespace std;

// Cellspace dimensions
#define WIDTH  100
#define HEIGHT 100

// Phase space to visualize
Phase phase[WIDTH][HEIGHT];
// Window and cell dimensions.
#define CELL_SIZE 6
GLint const win_width = WIDTH * CELL_SIZE;
GLint const win_height = HEIGHT * CELL_SIZE;

static shared_ptr<adevs::CellSpace<Phase, int>> cell_space = nullptr;
static shared_ptr<adevs::Simulator<CellEvent, int>> simulator = nullptr;
static bool initialized = false;


void drawSpace() {

    // Setup the background and main window for the grid
    if (!initialized) {
        glutUseLayer(GLUT_NORMAL);
        glClearColor(0.0, 0.0, 1.0, 1.0);
        glColor3f(1.0, 1.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, (float)win_width, 0.0, (float)win_height, 1.0, -1.0);
        initialized = false;
    }

    // Update each cell for the simulation
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            if (phase[x][y] == Alive) {
                GLint wx = CELL_SIZE * x;
                GLint wy = CELL_SIZE * y;
                glRecti(wx, wy, wx + CELL_SIZE, wy + CELL_SIZE);
            }
        }
    }
    glutSwapBuffers();
}


short int count_living_cells(int x, int y) {
    short int nalive = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int xx = (x + dx) % WIDTH;
            int yy = (y + dy) % HEIGHT;
            if (xx < 0) {
                xx = WIDTH - 1;
            }
            if (yy < 0) {
                yy = HEIGHT - 1;
            }
            if (phase[xx][yy] == Alive && !(xx == x && yy == y)) {
                nalive++;
            }
        }
    }
    return nalive;
}


void simulateSpace() {

    // Reset the space if everything has died
    if (cell_space == nullptr) {
        // Create the cell state variable array
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                if (rand() % 8 == 0) {
                    phase[x][y] = Alive;
                } else {
                    phase[x][y] = Dead;
                }
            }
        }

        // Create the cellspace model
        cell_space = make_shared<adevs::CellSpace<Phase, int>>(WIDTH, HEIGHT);
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                // Count the living neighbors
                short int nalive = count_living_cells(x, y);
                // Create the cell with its initial count of living neighbors
                cell_space->add(
                    make_shared<Cell>(x, y, WIDTH, HEIGHT, phase[x][y], nalive,
                                      &(phase[x][y])),
                    x, y);
            }
        }
        // Create a simulator for the model
        simulator = make_shared<adevs::Simulator<CellEvent, int>>(cell_space);
    }

    // If everything has died, then restart on the next call
    if (simulator->nextEventTime() != adevs_inf<int>()) {
        simulator->execNextEvent();
    }
    // Draw the updated display
    drawSpace();
}

int main(int argc, char** argv) {

    // Seed the random number generator
    srand(time(NULL));

    // Setup the display
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("glife");
    glutPositionWindow(0, 0);
    glutDisplayFunc(drawSpace);
    glutIdleFunc(simulateSpace);
    // Blocks until the window is closed
    glutMainLoop();

    return 0;
}

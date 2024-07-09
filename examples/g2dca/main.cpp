#include <GL/freeglut.h>
#include <GL/glut.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "Cell.h"
#include "adevs/adevs.h"
using namespace std;

// Cellspace dimensions
#define WIDTH  350
#define HEIGHT 350
// Phase space to visualize
Phase phase[WIDTH][HEIGHT];

// Window and cell dimensions.
#define CELL_SIZE 3
GLint const win_width = WIDTH * CELL_SIZE;
GLint const win_height = HEIGHT * CELL_SIZE;
string name;

void drawSpace() {
    static bool init = true;
    if (init) {
        init = false;
        glutUseLayer(GLUT_NORMAL);
        glClearColor(0.0, 0.0, 1.0, 1.0);
        glColor3f(1.0, 1.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, (float)win_width, 0.0, (float)win_height, 1.0, -1.0);
    }
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
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos2f(20, 20);
    glColor3f(1.0, 1.0, 1.0);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12,
                     reinterpret_cast<unsigned char const*>(name.c_str()));
    glutSwapBuffers();
}

void simulateSpace() {
    // Seed the random number generator
    srand(time(NULL));
    // Dynamic cellspace model and simulator
    static adevs::CellSpace<Phase, int>* cell_space = NULL;
    static adevs::Simulator<CellEvent, int>* sim = NULL;
    static uint8_t rule[RULE_ARRAY_LEN];
    static char winTitle[128];
    static int gen = 0;
    // Reset the space if everything has died
    if (cell_space == NULL) {
        // Create the cell state variable array
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                if (rand() % 256 == 0) {
                    phase[x][y] = Alive;
                } else {
                    phase[x][y] = Dead;
                }
            }
        }
        // Create the cellspace model
        ostringstream sin;
        sin << "0x";
        cell_space = new adevs::CellSpace<Phase, int>(WIDTH, HEIGHT);
        for (int i = 0; i < RULE_ARRAY_LEN; i++) {
            rule[i] = rand() % 256;
            sin << std::hex << std::setfill('0') << std::setw(2)
                << uint16_t(rule[i]);
        }
        name = sin.str();
        Cell::init_space(WIDTH, HEIGHT, rule);
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                cell_space->add(new Cell(x, y, phase[x][y], &(phase[x][y])), x,
                                y);
            }
        }
        // Create a simulator for the model
        sim = new adevs::Simulator<CellEvent, int>(cell_space);
        gen = 0;
        drawSpace();
        sprintf(winTitle, "Generation %d", gen);
        glutSetWindowTitle(winTitle);
        usleep(1000000);
    }
    // If everything has died, then restart on the next call
    if (sim->nextEventTime() == adevs_inf<int>() || sim->nextEventTime() > 60) {
        delete cell_space;
        delete sim;
        sim = NULL;
        cell_space = NULL;
    }
    // Run the next simulation step
    else {
        gen++;
        usleep(1000000);
        sim->execNextEvent();
    }
    // Draw the updated display
    sprintf(winTitle, "Generation %d", gen);
    glutSetWindowTitle(winTitle);
    drawSpace();
}

int main(int argc, char** argv) {
    // Setup the display
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("2D");
    glutPositionWindow(0, 0);
    glutDisplayFunc(drawSpace);
    glutIdleFunc(simulateSpace);
    glutMainLoop();
    // Done
    return 0;
}

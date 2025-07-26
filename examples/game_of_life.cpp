/**
 * This is a discrete event simulation that implements
 * Conway's Game of Life. The cellular automata is displayed
 * using OpenGL and the GLUT library.
 * 
 * using g++ you can compile this on the command line with
 * 
 * g++ -Wall -I../include -L.. game_of_life.cpp -lGL -lglut -ladevs
 */
#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "adevs/adevs.h"

using namespace std;

// Cellspace dimensions
#define WIDTH  100
#define HEIGHT 100

// Shortcuts for important types
using Location = std::pair<int,int>;
using Atomic = adevs::Atomic<Location,int>;
using Graph = adevs::Graph<Location,int>;
using Simulator = adevs::Simulator<Location,int>;
using Bag = std::list<adevs::PinValue<Location>>;

/**
 * This is a cell in the game of life.
 */
class Cell : public Atomic {
  public:

    /**
     * The cell space to visualize. This is also
     * loaded with the initial state of the cell
     * cell space.
     */
    static bool alive[WIDTH][HEIGHT];

    /// Utility for getting location of a neighbor
    Location neighbor(int dx, int dy) const {
        int nx = x+dx;
        int ny = y+dy;
        if (nx < 0) nx = WIDTH-1;
        if (ny < 0) ny = HEIGHT-1;
        if (nx >= WIDTH) nx = 0;
        if (ny >= HEIGHT) ny = 0;
        return Location(nx,ny);
    }
    /**
     * For the given location in the space. The value in the
     * alive array gives the initial state for the cell.
     */
    Cell(int x, int y) : Atomic(),x(x),y(y),nalive(0) {
        // Get the initial number of living neighbors
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                Location n(neighbor(i,j));
                if (alive[n.first][n.second]) nalive++;
            }
        }
    }

    int ta() {
        // Check if a state change should occur
        if (check_death_rule() || check_born_rule()) {
            return 1;
        }
        // Otherwise, do nothing
        return adevs_inf<int>();
    }

    void delta_int() {}

    void delta_ext(int, Bag const &xb) {
        // A neighbor changed state. Update the count of living neighbors
        for (auto update: xb) {
            if (alive[update.value.first][update.value.second]) nalive++;
            else nalive--;
            // We have eight neighbors
            assert(nalive >= 0 && nalive <= 8);
        }
    }

    void delta_conf(Bag const &xb) {
        delta_ext(0,xb);
    }

    void output_func(Bag &yb) {
        // Get our location as a pair that we can send to our neighbors
        Location location(x,y);
        // Update our appearance on the board
        alive[x][y] = (check_born_rule()) ? true : false;
        // Send an event to let our neighbors know we have changed state
        yb.push_back(adevs::PinValue<Location>(output,location));
    }

    // Pin for broadcasting changes of state
    const adevs::pin_t output;

  private:
    // location of the cell in the 2D space.
    const int x, y;
    // number of living neighbors.
    int nalive;

    /// Returns true if the cell will be born
    bool check_born_rule() const {
        return (!alive[x][y] && nalive == 3);
    }
    /// Return true if the cell will die
    bool check_death_rule() const {
        return (alive[x][y] && (nalive < 2 || nalive > 3));
    }
};

bool Cell::alive[WIDTH][HEIGHT];

// Window and cell dimensions for visualization
const int CELL_SIZE = 6;
const GLint win_width = WIDTH * CELL_SIZE;
const GLint win_height = HEIGHT * CELL_SIZE;

/**
 * Function to render the cell space to the display using
 * OpenGL and GLUT.
 */
void drawSpace() {
    static bool initialized = false;
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
            if (Cell::alive[x][y]) {
                GLint wx = CELL_SIZE * x;
                GLint wy = CELL_SIZE * y;
                glRecti(wx, wy, wx + CELL_SIZE, wy + CELL_SIZE);
            }
        }
    }
    glutSwapBuffers();
}

/**
 * Advance the state of the game each time the GLUT idle
 * function is activated
 */
void simulateSpace() {
    static std::shared_ptr<Cell> cells[WIDTH][HEIGHT];
    static Simulator* sim = nullptr;
    // Create a new simulator if we don't have one
    if (sim == nullptr) {
        // Create the initial state of the cell space
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
               Cell::alive[x][y] = (rand() % 8) == 0;
            }
        }
        // Create the simulation model
        auto graph = std::make_shared<Graph>();
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                cells[x][y] = std::make_shared<Cell>(x,y);
                graph->add_atomic(cells[x][y]);
            }
        }
        // Connect the cells to one another
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        if (i == 0 && j == 0) continue;
                        Location n(cells[x][y]->neighbor(i,j));
                        graph->connect(cells[x][y]->output,cells[n.first][n.second]);
                    }
                }
            }
        }
        // Create a simulator for the model
        sim = new Simulator(graph);
    }

    // Take the next step if there is more to do
    if (sim->nextEventTime() < adevs_inf<int>()) {
        sim->execNextEvent();
        // Draw the updated display
        drawSpace();
    } else {
        // Otherwise destroy the cell space. Well start again on the
        // next pass
        delete sim;
        sim = nullptr;
    }
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

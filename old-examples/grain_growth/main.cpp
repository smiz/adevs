#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include "cell.h"
#include "params.h"
#include "save_results.h"
using namespace std;

double tend;

void simulateSpace() {
    int* grains = new int[SIZE * SIZE];
    int k = 0;
    clock_t trun = clock();
    // Initialize the cell space
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            Cell::angle[i][j] = k++;
        }
    }
    // Create a simulator
    adevs::CellSpace<event_t>* cell_space =
        new adevs::CellSpace<event_t>(SIZE, SIZE);
    // Create a cell to go into each point of the cellspace
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            Cell* cell = new Cell(x, y);
            cell_space->add(cell, x, y);
        }
    }
    // Create a simulator for the model
    adevs::Simulator<CellEvent>* sim =
        new adevs::Simulator<CellEvent>(cell_space);
    // Run the simulation
    double tL = 0.0;
    while (sim->nextEventTime() < adevs_inf<double>() &&
           sim->nextEventTime() < tend) {
        tL = sim->nextEventTime();
        sim->execNextEvent();
    }
    trun = clock() - trun;
    cout << tL << " " << trun << " " << Cell::state_changes << " "
         << (double)trun / (double)Cell::state_changes << endl;
    // Record the outcome
    k = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grains[k] = Cell::angle[i][j];
            k++;
        }
    }
    save_results(grains, SIZE, SIZE, "des.png");
    delete sim;
    delete cell_space;
    delete[] grains;
}

int main(int argc, char** argv) {
    tend = atof(argv[1]);
    simulateSpace();
    // Done
    return 0;
}

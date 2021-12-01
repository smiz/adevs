#include <cassert>
#include <iostream>
#include <random>
#include <climits>
#include "save_results.h"
#include "params.h" // Includes SIZE and W
using namespace std;

// Random number generators
default_random_engine generator;
uniform_real_distribution<double> real_dist(0.0,1.0);
uniform_int_distribution<int> binary_dist(0,1);
// Cell space for the model
int angle[2][SIZE][SIZE];
// Index of present cell space (0 or 1) values
int idx = 0; 
// Grain value array for visualizing the final state
int grains[SIZE*SIZE];
// End time of the simulation
int tend;
// Count of state transitions performed
int state_changes = 0;

// Index into neighborhood coordinates
int neighbor[8][2] = 
{
	{1,0},
	{0,1},
	{-1,0},
	{0,-1},
	{1,-1},
	{-1,1},
	{-1,-1},
	{1,1}
};

// Is a coordinate inside the cell space?
bool inside(int x, int y)
{
	return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

// Calculate new state value for cell at x,y
void update_cell(int x, int y)
{
	int E = 0, En = 0, pick, px, py;
	state_changes++;
	// If nothing changes, propagate current state to next step
	angle[(idx+1)%2][x][y] = angle[idx][x][y];
	// Change grain orientation?
	if (real_dist(generator) > W)
		return;
	// Pick a random neighbor that is inside the space
	do
	{
		pick = rand()%8;
		px = x+neighbor[pick][0];
		py = y+neighbor[pick][1];
	}
	while (!inside(px,py));
	// Calculate E for current state and E for possible new state
	for (int i = 0; i < 8; i++)
	{
		int nx = x+neighbor[i][0];
		int ny = y+neighbor[i][1];
		if (inside(nx,ny))
		{
			E += angle[idx][x][y] != angle[idx][nx][ny];
			En += angle[idx][px][py] != angle[idx][nx][ny];
		}
	}
	// Accept the new state?
	if (En < E || (En == E && binary_dist(generator) == 0))
		angle[(idx+1)%2][x][y] = angle[idx][px][py];
}

void simulateSpace()
{
	// Randomize the initial state space values
	int k = 0;
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			angle[0][i][j] = angle[1][i][j] = k++;
	// Time at which simulation proper begins
	clock_t start = clock();
	// Simulate until tend
	for (int t = 0; t < tend; t++)
	{
		// Switch grids so that we calculate new values using old values
		idx = (idx+1)%2;
		for (int i = 0; i < SIZE; i++)
			for (int j = 0; j < SIZE; j++)
				update_cell(i,j);
	}
	// Report end time, clock ticks needed, and state changes calculated
	cout << tend << " " << clock()-start << " "<< state_changes << endl;
	// Record the outcome to a png file
	k = 0;
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			grains[k++] = angle[idx][i][j];
	save_results(grains,SIZE,SIZE,"ca.png");
}

int main(int argc, char** argv)
{
	// Get the simulation end time
	tend = int(atof(argv[1]));
	// Perform the simulation
	simulateSpace();
	return 0;
}


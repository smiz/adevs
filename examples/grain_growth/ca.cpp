#include <cassert>
#include <iostream>
#include <random>
#include <climits>
#include "save_results.h"
#include "params.h"
using namespace std;

default_random_engine generator;
uniform_real_distribution<double> distribution(0.0,1.0);
int idx = 0;
int angle[2][SIZE][SIZE];
int grains[SIZE*SIZE];
int tend, state_changes = 0;

static int neighbor[8][2] = 
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

void update_cell(int x, int y)
{
	int E = 0, En = 0, pick, px, py;
	state_changes++;
	angle[(idx+1)%2][x][y] = angle[idx][x][y];
	if (distribution(generator) > W)
		return;
	do
	{
		pick = rand()%8;
		px = x+neighbor[pick][0];
		py = y+neighbor[pick][1];
	}
	while (!(px >= 0 && px < SIZE && py >= 0 && py < SIZE));
	for (int i = 0; i < 8; i++)
	{
		int nx = x+neighbor[i][0];
		int ny = y+neighbor[i][1];
		if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
		{
			E += angle[idx][x][y] != angle[idx][nx][ny];
			En += angle[idx][px][py] != angle[idx][nx][ny];
		}
	}
	if (En < E || (En == E && rand()%2 == 0))
		angle[(idx+1)%2][x][y] = angle[idx][px][py];
}

void simulateSpace()
{
	int k = 0;
	srand(0);
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			angle[0][i][j] = angle[1][i][j] = k++;
	clock_t start = clock();
	for (int t = 0; t < tend; t++)
	{
		idx = (idx+1)%2;
		for (int i = 0; i < SIZE; i++)
		{
			for (int j = 0; j < SIZE; j++)
			{
				update_cell(i,j);
			}
		}
	}
	cout << tend << " " << clock()-start << " "<< state_changes << endl;
	k = 0;
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			grains[k] = angle[idx][i][j];
			k++;
		}
	}
	save_results(grains,SIZE,SIZE,"ca.png");
}

int main(int argc, char** argv)
{
	tend = int(atof(argv[1]));
	simulateSpace();
	return 0;
}


#include "adevs.h"
#include "Cell.h"
#include <iostream>
using namespace adevs;
using namespace std;

class Listener:
	public EventListener<car_t*>
{
	public:
		Listener()
		{
		}
		void outputEvent(Event<car_t*> x, double t)
		{
			Cell* model = dynamic_cast<Cell*>(x.model);
			#pragma omp critical
			{
				cout << "Car " << x.value->ID << " left cell " 
					<< model->getPos() << " @ t = " << t << endl;
			}
		}
		void stateChange(Atomic<car_t*>* model, double t)
		{
			Cell* cell = dynamic_cast<Cell*>(model);
			assert(t == cell->getTime());
			#pragma omp critical
			{
				cout << cell->getMsg() << endl;
			}
		}
		~Listener()
		{
		}
};

int main()
{
	SimpleDigraph<car_t*>* model = new SimpleDigraph<car_t*>();
	vector<Cell*> road;
	for (unsigned i = 0; i < 10; i++)
	{
		car_t* car = NULL;
		if (i == 0 || i == 5)
		{
			car = new car_t;
			car->ID = i;
			car->spd = MAX_SPEED/2.0;
			if (i == 0) car->spd = MAX_SPEED;
		}
		road.push_back(new Cell(i,car));
		model->add(road[i]);
	}
	for (unsigned i = 0; i < 9; i++)
	{
		model->couple(road[i],road[i+1]);
	}
	ParSimulator<car_t*>* sim = new ParSimulator<car_t*>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(2.0);
	sim->execUntil(100.0);
	delete sim;
	delete model;
	return 0;
}

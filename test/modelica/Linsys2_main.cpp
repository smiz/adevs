#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Linsys2.h"
#include <iostream>
using namespace std;
using namespace adevs;

void test_vars(Linsys2* model)
{
	for (int i = 0; i < 2; i++)
	{
		cout << model->get_time() << " ";
		cout << model->get_system_x(i,0) << " ";
		cout << model->get_system_x(i,1) << " ";
		cout << model->get_system_y(i,0) << " ";
		cout << model->get_system_y(i,1) << " ";
		cout << model->get_system_u(i,0) << " ";
		cout << model->get_system_u(i,1) << " ";
		cout << endl;
		assert(model->get_system_x(i,0) == model->get_system_y(i,0));
		assert(model->get_system_x(i,1) == model->get_system_y(i,1));
		assert(model->get_system_y((i+1)%2,0) == model->get_system_u(i,0));
		assert(model->get_system_y((i+1)%2,1) == model->get_system_u(i,1));
	}
}

void print_params(Linsys2* model)
{
	for (int i = 0; i < 2; i++)
	{
		for (int r = 0; r < 2; r++)
		{
			for (int c = 0; c < 2; c++)
			{
				cerr << model->get_system_A(i,r,c) << " ";
			}
			cerr << endl;
		}
		cerr << endl;
	}
	for (int i = 0; i < 2; i++)
	{
		for (int r = 0; r < 2; r++)
		{
			for (int c = 0; c < 2; c++)
			{
				cerr << model->get_system_B(i,r,c) << " ";
			}
			cerr << endl;
		}
		cerr << endl;
	}
	for (int i = 0; i < 2; i++)
	{
		for (int r = 0; r < 2; r++)
		{
			for (int c = 0; c < 2; c++)
			{
				cerr << model->get_system_C(i,r,c) << " ";
			}
			cerr << endl;
		}
		cerr << endl;
	}
}

int main()
{
	// Create the circuit
	Linsys2* model = new Linsys2();
	// Create an atomic model to simulate it
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		model,
		new rk_45<OMC_ADEVS_IO_TYPE>(model,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(model,1E-5));
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
	print_params(model);
	// Simulate 
	while (sim->nextEventTime() <= 100.0)
	{
		sim->execNextEvent();
		test_vars(model);
	}
	// Done, cleanup
	delete sim;
	delete hybrid_model;
	return 0;
}

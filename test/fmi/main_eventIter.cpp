#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
#include <cassert>
using namespace std;
using namespace adevs;

static const double epsilon = 1E-3;

class TestModel:
	public adevs::FMI<double>
{
	public:
		TestModel():
			adevs::FMI<double>(
					"eventIter",
					"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
					1,3,
					"event_tests/binaries/linux64/eventIter.so")
		{
		}
		double get_c() { return get_real(0); }
		double get_floorc() { return get_real(2); }
		int get_high() { return get_int(0); }
};

void print(TestModel* model)
{
	double cfloor = model->get_c()-floor(model->get_c());
	cout << model->get_time() << " "
		<< model->get_c() << " "
		<< model->get_floorc() << " "
		<< model->get_high() <<
		endl;
	assert(model->get_floorc() == cfloor);
	assert(cfloor <= 0.5+epsilon);
	assert(cfloor <= 0.25+epsilon || model->get_high()==1);
	assert(cfloor >= 0.25+epsilon || model->get_high()==0);
}

int main()
{
	TestModel* test_model = new TestModel();
	Hybrid<double>* hybrid_model =
		new Hybrid<double>(
		test_model,
		new corrected_euler<double>(test_model,1E-5,0.01),
		new discontinuous_event_locator<double>(test_model,1E-5));
        // Create the simulator
        Simulator<double>* sim =
			new Simulator<double>(hybrid_model);
		// Check initial values
		print(test_model);
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 10.0)
		{
			sim->execNextEvent();
			print(test_model);
		}
        delete sim;
		delete hybrid_model;
        return 0;
}

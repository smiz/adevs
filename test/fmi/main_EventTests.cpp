#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
using namespace std;

#define x_ref 0
#define y_ref 1
#define count_ref 0
#define x1_ref 6
#define v1_ref 2
#define v2_ref 3
#define v3_ref 4
#define v4_ref 5
#define w1_ref 6
#define w2_ref 7
#define w3_ref 8
#define w4_ref 9
#define epsilon 1E-6

void test_vars(adevs::FMI<int>* model)
{
	cout <<
		model->get_time() << " " <<
		model->get_int(count_ref) << " " <<
		model->get_real(x_ref) << " " <<
		model->get_real(y_ref) << " " <<
		endl;
	assert(fabs(model->get_real(x_ref)) <= 1.0+epsilon);
	assert(fabs(model->get_real(y_ref)) <= 1.0+epsilon);
	assert(model->get_int(count_ref) >= floor(model->get_time()*2.0)+1);
	if (model->get_real(x_ref)-model->get_real(x1_ref) > epsilon)
	{
		assert(model->get_bool(v1_ref));
		assert(model->get_bool(v2_ref));
		assert(!model->get_bool(v3_ref));
		assert(!model->get_bool(v4_ref));
	}
	// 2 x accounts for hysteresis
	else if (model->get_real(x_ref)-model->get_real(x1_ref) < -epsilon)
	{
		assert(!model->get_bool(v1_ref));
		assert(!model->get_bool(v2_ref));
		assert(model->get_bool(v3_ref));
		assert(model->get_bool(v4_ref));
	}
	// 2 x accounts for hysteresis
	if (model->get_real(x_ref)-model->get_real(y_ref) > epsilon)
	{
		assert(model->get_bool(w1_ref));
		assert(model->get_bool(w2_ref));
		assert(!model->get_bool(w3_ref));
		assert(!model->get_bool(w4_ref));
	}
	// 2 x accounts for hysteresis
	else if (model->get_real(x_ref)-model->get_real(y_ref) < -epsilon)
	{
		assert(!model->get_bool(w1_ref));
		assert(!model->get_bool(w2_ref));
		assert(model->get_bool(w3_ref));
		assert(model->get_bool(w4_ref));
	} 
}

int main()
{
	adevs::FMI<int>* fmi =
		new adevs::FMI<int>(
				"EventTests",
				"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
				2,8,
				"event_tests/binaries/linux64/EventTests.so",
				epsilon/10.0);
	adevs::corrected_euler<int>* solver1 = new adevs::corrected_euler<int>(fmi,epsilon/10.0,0.01);
	adevs::discontinuous_event_locator<int>* solver2 =
		new adevs::discontinuous_event_locator<int>(fmi,epsilon/10.0);
	adevs::Hybrid<int>* model =
		new adevs::Hybrid<int>(fmi,solver1,solver2);
	adevs::Simulator<int>* sim = new adevs::Simulator<int>(model);
	while (sim->nextEventTime() <= 5.0)
	{
		sim->execNextEvent();
		test_vars(fmi);
	}
	delete sim;
	delete model;
	return 0;
}

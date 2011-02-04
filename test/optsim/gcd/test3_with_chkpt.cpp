#include <iostream>
#include "adevs.h"
#include "state_saving_models.h"
#include "Listener.h"
using namespace std;

int main() 
{
	cout << "Test 3" << endl;
	gcd* c1 = new gcdWithStateSaving(10,2,1,false);
	gcd* c2 = new gcdWithStateSaving(10,2,1,false);
	genr* g = new genrWithStateSaving(1,1,true);
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	model->add(c1);
	model->add(c2);
	model->add(g);
	model->couple(g,g->signal,c1,c1->in);
	model->couple(c1,c1->out,c2,c2->in);
	adevs::ParSimulator<PortValue>* sim =
		new adevs::ParSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(100.0);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

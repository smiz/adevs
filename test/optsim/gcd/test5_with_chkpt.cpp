#include <iostream>
#include "adevs.h"
#include "state_saving_models.h"
#include "Listener.h"
using namespace std;

int main() 
{
	cout << "Test 5" << endl;
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	gcd* c = new gcdWithStateSaving(10,2,1,false);
	genr* g = new genrWithStateSaving(50,1000,true);
	model->add(c);
	model->add(g);
	model->couple(g,g->signal,c,c->in);
	model->couple(c,c->out,g,g->stop);
	adevs::ParSimulator<PortValue>* sim =
		new adevs::ParSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(60.0);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

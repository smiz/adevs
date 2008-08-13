#include <iostream>
#include "adevs.h"
#include "gcd.h"
#include "Listener.h"
using namespace std;

int main() 
{
	cout << "Test 1" << endl;
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	gcd* c = new gcd(10.0,2.0,1,false);
	genr* g = new genr(10.0,1,true);
	model->couple(g,g->signal,c,c->in);
	adevs::OptSimulator<PortValue>* sim =
		new adevs::OptSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(DBL_MAX);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

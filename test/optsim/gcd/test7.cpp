#include <iostream>
#include "adevs.h"
#include "gcd.h"
#include "Listener.h"
using namespace std;

int main () 
{
	cout << "Test 7" << endl;
	gcd* c = new gcd(10,2,1,false);
	gcd* g = new gcd(50,2,1000,true);
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	model->add(c);
	model->add(g);
	model->couple(g,g->signal,c,c->in);
	model->couple(c,c->out,g,g->stop);
	adevs::OptSimulator<PortValue>* sim = new adevs::OptSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(DBL_MAX);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

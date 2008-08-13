#include <iostream>
#include "adevs.h"
#include "gcd.h"
#include "Listener.h"
using namespace std;

int main() 
{
	cout << "Test 3" << endl;
	gcd* c1 = new gcd(10,2,1,false);
	gcd* c2 = new gcd(10,2,1,false);
	genr* g = new genr(1,1,true);
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	model->add(c1);
	model->add(c2);
	model->add(g);
	model->couple(g,g->signal,c1,c1->in);
	model->couple(c1,c1->out,c2,c2->in);
	adevs::OptSimulator<PortValue>* sim = new adevs::OptSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(DBL_MAX);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

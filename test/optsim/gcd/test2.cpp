#include <iostream>
#include "adevs.h"
#include "gcd.h"
#include "Listener.h"
using namespace std;

int main() 
{
	cout << "Test 2" << endl;
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	vector<double> pat;
	pat.push_back(0);
	pat.push_back(0);
	gcd* c = new gcd(10,2,1,false);
	genr* g = new genr(pat,2,true);
	model->add(c);
	model->add(g);
	model->couple(g,g->signal,c,c->in);
	adevs::OptSimulator<PortValue>* sim = new adevs::OptSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(DBL_MAX);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

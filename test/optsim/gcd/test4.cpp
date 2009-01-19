#include <iostream>
#include "adevs.h"
#include "gcd.h"
#include "Listener.h"
using namespace std;

int main () 
{
	cout << "Test 4" << endl;
	vector<double> pat;
	pat.push_back(0);
	pat.push_back(0);
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	gcd* c1 = new gcd(10,2,1,false);
	gcd* c2 = new gcd(10,2,1,false);
	genr* g = new genr(pat,2,true);
	model->add(c1);
	model->add(c2);
	model->add(g);
	model->couple(g,g->signal,c1,c1->in);
	model->couple(c1,c1->out,c2,c2->in);
	adevs::ParSimulator<PortValue>* sim = new adevs::ParSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(DBL_MAX);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}

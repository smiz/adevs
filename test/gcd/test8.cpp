#include <iostream>
#include "adevs.h"
#include "gcd.h"
using namespace std;

int main() 
{
	cout << "Test 8" << endl;
	vector<double> pat;
	pat.push_back(50);
	pat.push_back(0);
	adevs::Digraph<object*> model;
	gcd* c = new gcd(10,2,1,false);
	gcd* g = new gcd(pat,2,1000,true);
	model.add(c);
	model.add(g);
	model.couple(g,g->signal,c,c->in);
	model.couple(c,c->out,g,g->stop);
	adevs::Simulator<PortValue> sim(&model);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.execNextEvent();
	}
	cout << "Test done" << endl;
	return 0;
}

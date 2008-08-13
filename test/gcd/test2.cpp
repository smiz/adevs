#include <iostream>
#include "adevs.h"
#include "gcd.h"
using namespace std;

int main() 
{
	cout << "Test 2" << endl;
	adevs::Digraph<object*> model;
	vector<double> pat;
	pat.push_back(0);
	pat.push_back(0);
	gcd* c = new gcd(10,2,1,false);
	genr* g = new genr(pat,2,true);
	model.add(c);
	model.add(g);
	model.couple(g,g->signal,c,c->in);
	adevs::Simulator<PortValue> sim(&model);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.execNextEvent();
	}
	cout << "Test done" << endl;
	return 0;
}

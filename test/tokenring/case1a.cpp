#include <iostream>
#include <list>
#include "node.h"
using namespace std;

/**
  * This is the token ring case 1 test but it initializes the
  * simulator using a list of atomic components instead of
  * providing the coupled model.
  */
int main () 
{
	list<adevs::Devs<PortValue>*> active;
	adevs::Digraph<token_t*> model;
	node* n1 = new node(0,1,new token_t());
	node* n2 = new node(1,1,NULL);
	active.push_back(n1);
	active.push_back(n2);
	model.add(n1);
	model.add(n2);
	model.couple(n1,n1->out,n2,n2->in);
	model.couple(n2,n2->out,n1,n1->in);  
	adevs::Simulator<PortValue> sim(active);
	for (int i = 0; i < 10 && sim.nextEventTime() < DBL_MAX; i++)
	{
		cout << endl;
		sim.execNextEvent();
	}
	cout << endl;
	cout << "End of run!" << endl;
	return 0;
}
